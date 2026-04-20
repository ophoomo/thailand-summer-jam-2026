#include "renderer/msdf_font.h"
#include "stb/stb_image.h"
#include "utils/logger.h"
#include <nlohmann/json.hpp>

// ============================================================
// Public Methods
// ============================================================

bool MsdfFont::load(const nlohmann::json j, const uint8_t *raw, const int w, const int h)
{
    // Atlas metadata
    m_atlas.width = j["atlas"]["width"].get<int>();
    m_atlas.height = j["atlas"]["height"].get<int>();
    m_atlas.distanceRange = j["atlas"]["distanceRange"].get<float>();
    m_atlas.glyphSize = j["atlas"]["size"].get<float>();

    // Font metrics (all values in em units)
    m_metrics.emSize = j["metrics"]["emSize"].get<float>();
    m_metrics.lineHeight = j["metrics"]["lineHeight"].get<float>();
    m_metrics.ascender = j["metrics"]["ascender"].get<float>();
    m_metrics.descender = j["metrics"]["descender"].get<float>();

    // Glyph table
    for (const auto &g : j["glyphs"]) {
        GlyphInfo info;
        info.codepoint = g["unicode"].get<uint32_t>();
        info.advance = g["advance"].get<float>();

        if (g.contains("planeBounds") && g.contains("atlasBounds")) {
            const auto &pb = g["planeBounds"];
            info.planeBounds = {
                pb["left"].get<float>(),
                pb["top"].get<float>(),
                pb["right"].get<float>(),
                pb["bottom"].get<float>(),
            };

            const auto &ab = g["atlasBounds"];
            info.atlasBounds = {
                ab["left"].get<float>(),
                ab["top"].get<float>(),
                ab["right"].get<float>(),
                ab["bottom"].get<float>(),
            };
            info.hasBounds = true;
        }

        m_glyphs[info.codepoint] = info;
    }

    // Kerning pairs (optional)
    if (j.contains("kerning")) {
        for (const auto &k : j["kerning"]) {
            const uint32_t cp1 = k["unicode1"].get<uint32_t>();
            const uint32_t cp2 = k["unicode2"].get<uint32_t>();
            const float adv = k["advance"].get<float>();
            m_kerning[(static_cast<uint64_t>(cp1) << 32) | cp2] = adv;
        }
    }

    // ── Load atlas pixels (RGBA8) ─────────────────────────────────────────────
    if (w != m_atlas.width || h != m_atlas.height) {
        LOG_CORE_WARN("[MsdfFont] Atlas image size {}x{} != JSON {}x{}", w, h, m_atlas.width,
                      m_atlas.height);
        m_atlas.width = w;
        m_atlas.height = h;
    }

    m_pixels.assign(raw, raw + static_cast<size_t>(w) * h * 4);

    LOG_CORE_TRACE("[MsdfFont] Loaded {}×{}px, {} glyphs, {} kern pairs", w, h, m_glyphs.size(),
                   m_kerning.size());
    return true;
}

const GlyphInfo *MsdfFont::getGlyph(uint32_t codepoint) const
{
    auto it = m_glyphs.find(codepoint);
    return (it != m_glyphs.end()) ? &it->second : nullptr;
}

float MsdfFont::getKerning(uint32_t cp1, uint32_t cp2) const
{
    auto it = m_kerning.find((static_cast<uint64_t>(cp1) << 32) | cp2);
    return (it != m_kerning.end()) ? it->second : 0.0f;
}

uint32_t MsdfFont::utf8Next(const char *&p)
{
    const auto c = static_cast<uint8_t>(*p++);
    if (c < 0x80u)
        return c;
    if (c < 0xE0u) {
        return ((c & 0x1Fu) << 6) | (static_cast<uint8_t>(*p++) & 0x3Fu);
    }
    if (c < 0xF0u) {
        const uint32_t b1 = static_cast<uint8_t>(*p++) & 0x3Fu;
        const uint32_t b2 = static_cast<uint8_t>(*p++) & 0x3Fu;
        return ((c & 0x0Fu) << 12) | (b1 << 6) | b2;
    }
    const uint32_t b1 = static_cast<uint8_t>(*p++) & 0x3Fu;
    const uint32_t b2 = static_cast<uint8_t>(*p++) & 0x3Fu;
    const uint32_t b3 = static_cast<uint8_t>(*p++) & 0x3Fu;
    return ((c & 0x07u) << 18) | (b1 << 12) | (b2 << 6) | b3;
}
