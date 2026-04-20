
#include "renderer/ox_renderer.h"
#include "renderer/camera.h"
#include "renderer/msdf_font.h"
#include "renderer/renderer_interface.h"
#include "renderer/text_effect.h"
#include "renderer/vulkan/vulkan_renderer.h"
#include "utils/logger.h"
#include <glm/glm.hpp>
#include <memory>

// ============================================================
// Construction / destruction
// ============================================================

OxRenderer::OxRenderer(RendererAPI api, std::shared_ptr<Window> window)
{
    this->m_window = window;
    switch (api) {
    case RendererAPI::Vulkan:
        this->m_renderer = std::make_unique<VulkanRenderer>(this->m_window);
        break;
    default:
        LOG_CORE_ERROR("OxRenderer: Unsupported RendererAPI specified: {}", static_cast<int>(api));
        throw std::runtime_error("OxRenderer: Unsupported RendererAPI");
    }
    this->ResetCamera();
}

OxRenderer::~OxRenderer() = default;

// ============================================================
// Public Methods
// ============================================================

void OxRenderer::resize()
{
    int w = m_window->getWidth();
    int h = m_window->getHeight();

    if (w <= 0 || h <= 0) {
        LOG_CORE_WARN("Window Size Width or Height <= 0");
        return;
    }

    if (this->m_renderer) {
        this->m_renderer->resize(w, h);
    }
    this->m_window->resetResizedFlag();
}

void OxRenderer::initImGui()
{
    this->m_renderer->initImGui(m_window->get());
}

void OxRenderer::oxBegin()
{
    this->m_cmdBuf.clear();
    this->m_drawList.clear();
    this->m_renderer->BeginFrame();
}

void OxRenderer::oxEnd()
{
    this->flushAll();
    this->m_renderer->EndFrame();
}

void OxRenderer::oxDrawRectangle(float x, float y, float width, float height, Color color,
                                 int32_t layer)
{
    if (m_cmdBuf.count >= MAX_COMMANDS) [[unlikely]] {
        LOG_CORE_WARN("[OxRenderer]: command buffer full — draw call dropped");
        return;
    }
    const uint32_t i = m_cmdBuf.count++;

    m_cmdBuf.positions[i] = {x, y};
    m_cmdBuf.sizes[i] = {width, height};
    m_cmdBuf.origins[i] = {0.f, 0.f};
    m_cmdBuf.rotations[i] = 0.f;
    m_cmdBuf.colors[i] = color.Pack();
    m_cmdBuf.texIndices[i] = 0;
    m_cmdBuf.uvMins[i] = {0.f, 0.f};
    m_cmdBuf.uvMaxs[i] = {1.f, 1.f};
    m_cmdBuf.perm[i] = i;

    m_cmdBuf.keys[i] =
        DrawCommandBuffer::MakeSortKey(static_cast<uint8_t>(std::clamp(layer, 0, 255)),
                                       static_cast<uint8_t>(PipelineID::AlphaQuad), 0, y);

    // Key layout: layer(8) | type=0(8) | 0(16) | y_depth(32)
    const uint64_t key = (static_cast<uint64_t>(std::clamp(layer, 0, 255)) << 56) |
                         std::bit_cast<uint32_t>(std::max(0.f, y));
    m_drawList.push_back({key, 0, i});
}

void OxRenderer::oxDrawCircle(float cx, float cy, float radius, Color color, int32_t layer)
{
    if (m_cmdBuf.count >= MAX_COMMANDS) [[unlikely]] {
        LOG_CORE_WARN("[OxRenderer]: command buffer full — draw call dropped");
        return;
    }
    const uint32_t i = m_cmdBuf.count++;

    m_cmdBuf.positions[i] = {cx - radius, cy - radius};
    m_cmdBuf.sizes[i] = {radius * 2.f, radius * 2.f};
    m_cmdBuf.origins[i] = {0.f, 0.f};
    m_cmdBuf.rotations[i] = 0.f;
    m_cmdBuf.colors[i] = color.Pack();
    m_cmdBuf.texIndices[i] = 0;
    m_cmdBuf.uvMins[i] = {0.f, 0.f};
    m_cmdBuf.uvMaxs[i] = {1.f, 1.f};
    m_cmdBuf.perm[i] = i;

    // type=3 in bits 55-48 ensures circles sort together, after rects/sprites/text
    const uint64_t key = (static_cast<uint64_t>(std::clamp(layer, 0, 255)) << 56) | (3ULL << 48) |
                         std::bit_cast<uint32_t>(std::max(0.f, cy));
    m_drawList.push_back({key, 3, i});
}

void OxRenderer::oxDrawText(float x, float y, const char *text, float size, Color color,
                            TextEffect effect, int32_t layer)
{
    if (!text || *text == '\0')
        return;
    const uint32_t idx = static_cast<uint32_t>(m_textCommands.size());
    this->m_textCommands.push_back({text, x, y, size, color.Pack(), layer, effect});
    // Key layout: layer(8) | type=2(8) | 0(48)
    const uint64_t key = (static_cast<uint64_t>(std::clamp(layer, 0, 255)) << 56) | (2ULL << 48);
    m_drawList.push_back({key, 2, idx});
}

void OxRenderer::oxDrawSprite(float x, float y, float w, float h, const std::string texture_name,
                              Color tint, int32_t layer)
{
    auto handle = this->m_texture_cached[texture_name];
    if (handle == INVALID_TEXTURE)
        return;
    const uint32_t idx = static_cast<uint32_t>(m_spriteCommands.size());
    this->m_spriteCommands.push_back({x, y, w, h, handle, tint.Pack(), layer});
    // Key layout: layer(8) | type=1(8) | texture_id(16) | 0(32)  — groups same-texture sprites
    const uint64_t key = (static_cast<uint64_t>(std::clamp(layer, 0, 255)) << 56) | (1ULL << 48) |
                         (static_cast<uint64_t>(handle & 0xFFFF) << 32);
    m_drawList.push_back({key, 1, idx});
}

void OxRenderer::SetCamera(const Camera2D &cam)
{
    this->m_camera = cam;
}

void OxRenderer::ResetCamera()
{
    this->m_camera = Camera2D{};
    this->m_camera.UpdateMatrices(static_cast<float>(m_window->getWidth()),
                                  static_cast<float>(m_window->getHeight()));
}

void OxRenderer::loadFont(const nlohmann::json j, const uint8_t *raw, const int w, const int h)
{
    this->m_font = std::make_unique<MsdfFont>();
    this->m_font->load(j, raw, w, h);
    const TextureHandle tex = this->m_renderer->createTexture(
        this->m_font->pixels(), this->m_font->atlas().width, this->m_font->atlas().height);

    if (tex == INVALID_TEXTURE) {
        LOG_CORE_ERROR("[OxRenderer] createTexture failed for font atlas");
        return;
    };

    this->m_fontTex = tex;

    const float dr = this->m_font->atlas().distanceRange;
    this->m_renderer->setMsdfUnitRange(dr / static_cast<float>(this->m_font->atlas().width),
                                       dr / static_cast<float>(this->m_font->atlas().height));

    LOG_CORE_TRACE("[OxRenderer] Font loaded, id={}", tex);
}

float OxRenderer::measureText(const char *text, float size) const
{
    const float scale = size / this->m_font->metrics().emSize;
    float width = 0.0f;
    uint32_t prevCp = 0;
    const char *p = text;
    while (*p) {
        const uint32_t cp = this->m_font->utf8Next(p);
        const GlyphInfo *g = this->m_font->getGlyph(cp);
        if (!g)
            continue;
        width += (g->advance + this->m_font->getKerning(prevCp, cp)) * scale;
        prevCp = cp;
    }
    return width;
}

TextureHandle OxRenderer::createTexture(const std::string texture_name, const uint8_t *pixels,
                                        int width, int height)
{
    auto id = this->m_renderer->createTexture(pixels, width, height);
    this->m_texture_cached[texture_name] = id;
    return id;
}

void OxRenderer::freeTexture(const std::string texture_name)
{
    auto it = this->m_texture_cached.find(texture_name);
    if (it == this->m_texture_cached.end()) {
        return;
    }
    auto handle = it->second;
    if (handle != INVALID_TEXTURE) {
        this->m_renderer->destroyTexture(handle);
    }
    this->m_texture_cached.erase(it);
}

void OxRenderer::waitIdle()
{
    this->m_renderer->waitIdle();
}

// ============================================================
// Private Methods
// ============================================================

void OxRenderer::writeQuadVertices(uint32_t i, Vertex2D *out) const
{
    // Generate the four corners of the quad in world space.
    // No rotation applied here — add a 2D rotation matrix if oxDrawSprite
    // exposes a rotation parameter in the future.
    const glm::vec2 pos = m_cmdBuf.positions[i];
    const glm::vec2 sz = m_cmdBuf.sizes[i];
    const uint32_t col = m_cmdBuf.colors[i];
    const float slot = static_cast<float>(m_cmdBuf.texIndices[i]);
    const glm::vec2 uvMin = m_cmdBuf.uvMins[i];
    const glm::vec2 uvMax = m_cmdBuf.uvMaxs[i];

    // Winding order: TL → TR → BR → BL  (matches index buffer 0,1,2, 0,2,3)
    out[0] = {pos.x, pos.y, uvMin.x, uvMin.y, col, slot};               // TL
    out[1] = {pos.x + sz.x, pos.y, uvMax.x, uvMin.y, col, slot};        // TR
    out[2] = {pos.x + sz.x, pos.y + sz.y, uvMax.x, uvMax.y, col, slot}; // BR
    out[3] = {pos.x, pos.y + sz.y, uvMin.x, uvMax.y, col, slot};        // BL
}

void OxRenderer::flushSingleTextCommand(const TextCommand &cmd)
{
    this->m_textVertices.clear();

    const float aw = static_cast<float>(this->m_font->atlas().width);
    const float ah = static_cast<float>(this->m_font->atlas().height);
    const float scale = cmd.size / this->m_font->metrics().emSize;

    float cursorX = cmd.x;
    uint32_t prevCp = 0;
    const char *p = cmd.text.c_str();

    while (*p) {
        const uint32_t cp = this->m_font->utf8Next(p);
        if (cp == '\n') {
            cursorX = cmd.x;
            prevCp = 0;
            continue;
        }
        const GlyphInfo *g = this->m_font->getGlyph(cp);
        if (!g) {
            prevCp = cp;
            continue;
        }
        cursorX += this->m_font->getKerning(prevCp, cp) * scale;
        prevCp = cp;
        if (g->hasBounds) {
            const float sl = cursorX + g->planeBounds.left * scale;
            const float sr = cursorX + g->planeBounds.right * scale;
            const float st = cmd.y + g->planeBounds.top * scale;
            const float sb = cmd.y + g->planeBounds.bottom * scale;
            const float ul = g->atlasBounds.left / aw;
            const float ur = g->atlasBounds.right / aw;
            const float ut = g->atlasBounds.top / ah;
            const float ub = g->atlasBounds.bottom / ah;
            this->m_textVertices.push_back({sl, st, ul, ut, cmd.color, 0.0f});
            this->m_textVertices.push_back({sr, st, ur, ut, cmd.color, 0.0f});
            this->m_textVertices.push_back({sr, sb, ur, ub, cmd.color, 0.0f});
            this->m_textVertices.push_back({sl, sb, ul, ub, cmd.color, 0.0f});
        }
        cursorX += g->advance * scale;
    }

    if (!this->m_textVertices.empty()) {
        this->m_renderer->SubmitTextVertices(this->m_textVertices.data(),
                                             static_cast<uint32_t>(this->m_textVertices.size()) / 4,
                                             this->m_fontTex, toGPU(cmd.effect));
        this->m_textVertices.clear();
    }
}

void OxRenderer::flushAll()
{
    if (this->m_drawList.empty()) {
        this->m_cmdBuf.clear();
        return;
    }

    // Single sort over all draw types — primary key is layer (bits 63-56),
    // then type (bits 55-48): rect(0) < sprite(1) < text(2),
    // then texture id for sprite batching.
    std::sort(this->m_drawList.begin(), this->m_drawList.end(),
              [](const DrawEntry &a, const DrawEntry &b) { return a.key < b.key; });

    uint8_t curType = 0xFF; // 0xFF = nothing pending
    TextureHandle curTex = INVALID_TEXTURE;
    uint32_t rectCount = 0;
    PipelineID curQuadPipeline = PipelineID::AlphaQuad;
    this->m_spriteVertices.clear();

    auto flushQuads = [&]() {
        if (rectCount == 0)
            return;
        this->m_renderer->SubmitVertices(this->m_vertexScratch.data(), rectCount, curQuadPipeline);
        rectCount = 0;
    };

    auto flushSprites = [&]() {
        if (this->m_spriteVertices.empty())
            return;
        this->m_renderer->SubmitSpriteVertices(
            this->m_spriteVertices.data(), static_cast<uint32_t>(this->m_spriteVertices.size()) / 4,
            curTex);
        this->m_spriteVertices.clear();
    };

    auto flushCurrent = [&]() {
        if (curType == 0 || curType == 3)
            flushQuads();
        else if (curType == 1)
            flushSprites();
    };

    for (const DrawEntry &entry : this->m_drawList) {
        if (entry.type == 0) { // rect
            if (curType != 0) {
                flushCurrent();
                curType = 0;
                curQuadPipeline = PipelineID::AlphaQuad;
            }
            this->writeQuadVertices(entry.idx, &this->m_vertexScratch[rectCount * 4]);
            if (++rectCount >= MAX_COMMANDS)
                flushQuads();

        } else if (entry.type == 1) { // sprite
            const SpriteCommand &cmd = this->m_spriteCommands[entry.idx];
            if (curType != 1) {
                flushCurrent();
                curType = 1;
                curTex = INVALID_TEXTURE;
            }
            if (cmd.texture != curTex && curTex != INVALID_TEXTURE)
                flushSprites();
            curTex = cmd.texture;
            const float r = cmd.x + cmd.w;
            const float b = cmd.y + cmd.h;
            this->m_spriteVertices.push_back({cmd.x, cmd.y, 0.f, 0.f, cmd.color, 0.f});
            this->m_spriteVertices.push_back({r, cmd.y, 1.f, 0.f, cmd.color, 0.f});
            this->m_spriteVertices.push_back({r, b, 1.f, 1.f, cmd.color, 0.f});
            this->m_spriteVertices.push_back({cmd.x, b, 0.f, 1.f, cmd.color, 0.f});

        } else if (entry.type == 3) { // circle
            if (curType != 3) {
                flushCurrent();
                curType = 3;
                curQuadPipeline = PipelineID::Circle;
            }
            this->writeQuadVertices(entry.idx, &this->m_vertexScratch[rectCount * 4]);
            if (++rectCount >= MAX_COMMANDS)
                flushQuads();

        } else { // text
            flushCurrent();
            curType = 0xFF;
            this->flushSingleTextCommand(this->m_textCommands[entry.idx]);
        }
    }

    flushCurrent();

    this->m_drawList.clear();
    this->m_spriteCommands.clear();
    this->m_textCommands.clear();
    this->m_cmdBuf.clear();
}
