#ifndef A55D7456_5D75_4AE5_A1FC_B8890AF39587
#define A55D7456_5D75_4AE5_A1FC_B8890AF39587

#include "core/window.h"
#include "draw_command_buffer.h"
#include "renderer/camera.h"
#include "renderer/color.h"
#include "renderer/msdf_font.h"
#include "renderer/renderer_interface.h"
#include "renderer/text_effect.h"
#include "renderer/vertex.h"
#include "sprite_command.h"
#include "text_command.h"
#include <memory>
#include <nlohmann/json.hpp>

class OxRenderer
{
  public:
    OxRenderer(RendererAPI api, std::shared_ptr<Window> window);
    ~OxRenderer();

    void resize();
    void initImGui();

    void oxBegin();
    void oxEnd();

    void oxDrawRectangle(float x, float y, float width, float height, Color color = Color::White(),
                         int32_t layer = 0);
    void oxDrawCircle(float cx, float cy, float radius, Color color = Color::White(),
                      int32_t layer = 0);
    void oxDrawSprite(float x, float y, float w, float h, const std::string texture_name,
                      Color tint, int32_t layer = 0);
    void oxDrawText(float x, float y, const char *text, float size, Color color,
                    TextEffect effect = TextEffect::None(), int32_t layer = 0);

    TextureHandle createTexture(const std::string texture_name, const uint8_t *pixels, int width,
                                int height);
    void freeTexture(const std::string texture_name);
    void waitIdle();

    void SetCamera(const Camera2D &cam);
    void ResetCamera();

    void loadFont(const nlohmann::json j, const uint8_t *raw, const int w, const int h);
    float measureText(const char *text, float size) const;
    const MsdfFontMetrics &fontMetrics() const
    {
        return m_font->metrics();
    }

  private:
    struct DrawEntry
    {
        uint64_t key;
        uint8_t type; // 0 = rect, 1 = sprite, 2 = text
        uint32_t idx; // index into the respective command array
    };

    void flushAll();
    void flushSingleTextCommand(const TextCommand &cmd);
    void writeQuadVertices(uint32_t i, Vertex2D *out) const;

    std::unordered_map<std::string, TextureHandle> m_texture_cached;
    std::unique_ptr<RendererInterface> m_renderer;
    std::shared_ptr<Window> m_window;
    Camera2D m_camera;

    DrawCommandBuffer m_cmdBuf;
    std::array<Vertex2D, MAX_VERTICES> m_vertexScratch;

    std::vector<DrawEntry> m_drawList;

    std::vector<TextCommand> m_textCommands;
    std::vector<Vertex2D> m_textVertices;

    std::vector<Vertex2D> m_spriteVertices;
    std::vector<SpriteCommand> m_spriteCommands;

    std::unique_ptr<MsdfFont> m_font;
    TextureHandle m_fontTex;
};

#endif /* A55D7456_5D75_4AE5_A1FC_B8890AF39587 */
