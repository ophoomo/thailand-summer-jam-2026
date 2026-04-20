#ifndef DCC65C9C_B9CD_43A7_9617_6C2DD9B220B9
#define DCC65C9C_B9CD_43A7_9617_6C2DD9B220B9

#include <cstdint>

// ============================================================
// Color (RGBA 8-bit)
// ============================================================
struct Color
{
    uint8_t r = 255, g = 255, b = 255, a = 255;

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    // Pack as little-endian RGBA for Vertex2D::color.
    // Hardware reads this as VK_FORMAT_R8G8B8A8_UNORM → vec4(r,g,b,a)/255.
    [[nodiscard]] constexpr uint32_t Pack() const
    {
        return (uint32_t(a) << 24) | (uint32_t(b) << 16) | (uint32_t(g) << 8) | uint32_t(r);
    }

    static constexpr Color White()
    {
        return {255, 255, 255, 255};
    }
    static constexpr Color Black()
    {
        return {0, 0, 0, 255};
    }
    static constexpr Color Red()
    {
        return {255, 0, 0, 255};
    }
    static constexpr Color Green()
    {
        return {0, 255, 0, 255};
    }
    static constexpr Color Blue()
    {
        return {0, 0, 255, 255};
    }
    static constexpr Color Yellow()
    {
        return {255, 255, 0, 255};
    }
    static constexpr Color Transparent()
    {
        return {0, 0, 0, 0};
    }

    // Convenience: lerp between two colours
    [[nodiscard]] static Color Lerp(Color a, Color b, float t)
    {
        auto lerp = [t](uint8_t x, uint8_t y) -> uint8_t {
            return static_cast<uint8_t>(x + (y - x) * t);
        };
        return {lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b), lerp(a.a, b.a)};
    }
};

#endif /* DCC65C9C_B9CD_43A7_9617_6C2DD9B220B9 */
