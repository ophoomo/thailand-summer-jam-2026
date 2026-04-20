#ifndef A1B2C3D4_TEXT_EFFECT_H
#define A1B2C3D4_TEXT_EFFECT_H

#include "renderer/color.h"
#include <cstdint>

enum class TextEffectType : int32_t { None = 0, Outline = 1, Glow = 2 };

struct TextEffect
{
    TextEffectType type = TextEffectType::None;
    Color color = Color::Black();
    float param0 = 0.15f; // outline: width in SDF units; glow: range (0.0–0.5)
    float param1 = 1.0f;  // glow: strength multiplier

    static constexpr TextEffect None() { return {}; }
    static constexpr TextEffect Outline(Color c, float width = 0.15f)
    {
        return {TextEffectType::Outline, c, width, 1.0f};
    }
    static constexpr TextEffect Glow(Color c, float range = 0.3f, float strength = 1.0f)
    {
        return {TextEffectType::Glow, c, range, strength};
    }
};

// POD layout pushed to GPU at push-constant offset 16 (fragment stage).
// Must match the cbuffer in msdf_text.frag.slang exactly.
struct TextEffectGPU
{
    float color[4];   // rgba [0,1]
    float param0;
    float param1;
    int32_t type;
    int32_t _pad = 0;
};

inline TextEffectGPU toGPU(const TextEffect &e)
{
    TextEffectGPU g{};
    g.color[0] = e.color.r / 255.0f;
    g.color[1] = e.color.g / 255.0f;
    g.color[2] = e.color.b / 255.0f;
    g.color[3] = e.color.a / 255.0f;
    g.param0 = e.param0;
    g.param1 = e.param1;
    g.type = static_cast<int32_t>(e.type);
    return g;
}

#endif /* A1B2C3D4_TEXT_EFFECT_H */
