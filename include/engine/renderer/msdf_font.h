#ifndef A2A10E0C_9E1B_4C87_8357_41018665D804
#define A2A10E0C_9E1B_4C87_8357_41018665D804

#include "nlohmann/json_fwd.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>

// ─── GlyphBounds ─────────────────────────────────────────────────────────────
//  Used for both planeBounds (normalised em units) and atlasBounds (pixels).
//  Y-axis follows msdf-atlas-gen's -yorigin top convention (top < bottom).
struct GlyphBounds
{
    float left = 0, top = 0, right = 0, bottom = 0;
};

// ─── GlyphInfo ───────────────────────────────────────────────────────────────
//  One entry per Unicode codepoint loaded from the atlas JSON.
struct GlyphInfo
{
    uint32_t codepoint = 0;
    float advance = 0.0f;    // em units; multiply by fontSize to get pixels
    GlyphBounds planeBounds; // normalised quad corners (relative to baseline)
    GlyphBounds atlasBounds; // pixel coords inside the atlas image
    bool hasBounds = false;  // false for whitespace-only glyphs
};

// ─── MsdfFontMetrics ─────────────────────────────────────────────────────────
struct MsdfFontMetrics
{
    float emSize = 1.0f;
    float lineHeight = 1.2f; // in em units
    float ascender = 0.8f;   // distance above baseline (positive)
    float descender = -0.2f; // distance below baseline (negative)
};

// ─── MsdfAtlasInfo ───────────────────────────────────────────────────────────
struct MsdfAtlasInfo
{
    int width = 0;
    int height = 0;
    float distanceRange = 4.0f; // pxRange used at generation time
    float glyphSize = 48.0f;    // target cell size in pixels
};

// ─── MsdfFont ────────────────────────────────────────────────────────────────
//  Holds the CPU-side font data parsed from msdf-atlas-gen output (JSON + PNG).
//  This class owns the atlas pixel data; the GPU texture is managed separately
//  by the renderer (OxRenderer::loadFont returns a handle after upload).
class MsdfFont
{
  public:
    MsdfFont() = default;
    ~MsdfFont() = default;

    MsdfFont(const MsdfFont &) = delete;
    MsdfFont &operator=(const MsdfFont &) = delete;
    MsdfFont(MsdfFont &&) = default;
    MsdfFont &operator=(MsdfFont &&) = default;
    uint32_t utf8Next(const char *&p);

    // Load glyph layout from jsonPath and atlas pixels from pngPath.
    // Returns false and logs an error on failure.
    bool load(const nlohmann::json j, const uint8_t *raw, const int w, const int h);

    [[nodiscard]] const GlyphInfo *getGlyph(uint32_t codepoint) const;
    [[nodiscard]] float getKerning(uint32_t cp1, uint32_t cp2) const;

    [[nodiscard]] const MsdfFontMetrics &metrics() const
    {
        return m_metrics;
    }
    [[nodiscard]] const MsdfAtlasInfo &atlas() const
    {
        return m_atlas;
    }

    // Raw RGBA8 atlas pixels (atlasWidth × atlasHeight × 4 bytes).
    [[nodiscard]] const uint8_t *pixels() const
    {
        return m_pixels.data();
    }
    [[nodiscard]] bool loaded() const
    {
        return m_atlas.width > 0;
    }

  private:
    MsdfFontMetrics m_metrics;
    MsdfAtlasInfo m_atlas;
    std::unordered_map<uint32_t, GlyphInfo> m_glyphs;
    std::unordered_map<uint64_t, float> m_kerning; // key = (cp1 << 32) | cp2
    std::vector<uint8_t> m_pixels;
};

#endif /* A2A10E0C_9E1B_4C87_8357_41018665D804 */
