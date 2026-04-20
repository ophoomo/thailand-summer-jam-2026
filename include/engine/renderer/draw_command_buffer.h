#ifndef C84171B0_2AFE_4CB4_A467_A413DB302DB4
#define C84171B0_2AFE_4CB4_A467_A413DB302DB4

#include "glm/ext/vector_float2.hpp"
#include <bit>
#include <cstdint>

static constexpr uint32_t MAX_COMMANDS = 16384;
static constexpr uint32_t MAX_VERTICES = MAX_COMMANDS * 4;
static constexpr uint32_t MAX_INDICES = MAX_COMMANDS * 6;
static constexpr uint32_t MAX_TEXTURE_SLOTS = 8;
static constexpr uint32_t MAX_TEXTURES = 1024;
static constexpr uint32_t MAX_FONTS = 32;
static constexpr uint32_t MAX_SHADERS = 64;

struct DrawCommandBuffer
{
    uint32_t count = 0; // live command count; reset to 0 each frame via clear()

    // ── Sort phase ───────────────────────────────────────────────────────────
    uint64_t keys[MAX_COMMANDS]; // encoded sort keys (see MakeSortKey)
    uint32_t perm[MAX_COMMANDS]; // indirect index array: perm[i] = original slot

    // ── Payload (read in sorted perm[] order during vertex write) ────────────
    glm::vec2 positions[MAX_COMMANDS]; // world-space top-left corner
    glm::vec2 sizes[MAX_COMMANDS];     // width / height in world units
    glm::vec2 origins[MAX_COMMANDS];   // local-space pivot; (0,0) = top-left
    float rotations[MAX_COMMANDS];     // radians, CCW positive
    uint32_t colors[MAX_COMMANDS];     // packed RGBA8 (see Color::Pack())
    uint16_t texIndices[MAX_COMMANDS]; // index into the bound texture array; 0 = white
    glm::vec2 uvMins[MAX_COMMANDS];    // atlas sub-rect bottom-left  (default {0,0})
    glm::vec2 uvMaxs[MAX_COMMANDS];    // atlas sub-rect top-right    (default {1,1})

    // ── Sort key factory ─────────────────────────────────────────────────────
    //  depth must be in [0, +∞) for the bit-cast trick to preserve ordering.
    //  In a top-down 2D scene, world-space Y is a natural depth approximation.
    [[nodiscard]] static constexpr uint64_t MakeSortKey(uint8_t layer, uint8_t pipelineId,
                                                        uint16_t textureId, float depth) noexcept
    {
        const uint32_t depthBits = std::bit_cast<uint32_t>(depth);
        return (static_cast<uint64_t>(layer) << 56) | (static_cast<uint64_t>(pipelineId) << 48) |
               (static_cast<uint64_t>(textureId) << 32) | static_cast<uint64_t>(depthBits);
    }

    void clear() noexcept
    {
        count = 0;
    }
};

#endif /* C84171B0_2AFE_4CB4_A467_A413DB302DB4 */
