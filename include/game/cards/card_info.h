#ifndef B7E4C2A1_9F3D_4B88_A1E5_0D7C6F2E8B34
#define B7E4C2A1_9F3D_4B88_A1E5_0D7C6F2E8B34

#include <cstdint>
#include <string>

struct CardInfo
{
    int id = 0;
    std::string lua_id; // matches the "id" key in the Lua manifest
    std::string name;
    std::string inner;
    std::string detail;
    std::string type; // "attack" | "skill" | "power"
    uint8_t cost = 0;
    bool show = true;
    std::string art{"card_skill"}; // texture key: "card_white" | "card_dark" | "card_skill"

    // ── Battle mechanics (loaded from Lua) ────────────────────────────────────
    int32_t damage = 0;
    int32_t block = 0;
    int32_t heal = 0;
    int8_t apply_vulnerable = 0;
    int8_t apply_weak = 0;
    int8_t apply_strength = 0;
    bool exhaust = false;
};

#endif /* B7E4C2A1_9F3D_4B88_A1E5_0D7C6F2E8B34 */
