#ifndef BATTLE_ENEMY_INFO_H
#define BATTLE_ENEMY_INFO_H

#include "game/battle/components.h"
#include <string>
#include <vector>

namespace battle {

// ─────────────────────────────────────────────────────────────────────────────
//  AIPatternInfo — one step in an enemy's action cycle, loaded from Lua.
//  Converted from the "action" string to IntentType at load time so the
//  hot path never does string comparisons.
// ─────────────────────────────────────────────────────────────────────────────

struct AIPatternInfo {
    IntentType action{IntentType::ATTACK};
    int32_t    damage{0};
    int32_t    block{0};
    int32_t    times{1};
    int8_t     apply_vulnerable{0};
    int8_t     apply_weak{0};
};

// ─────────────────────────────────────────────────────────────────────────────
//  EnemyInfo — fully parsed, runtime-ready enemy definition
// ─────────────────────────────────────────────────────────────────────────────

struct EnemyInfo {
    std::string                lua_id;
    std::string                name;
    EnemyType                  type{EnemyType::SKELETON};
    int32_t                    base_hp{10};
    int32_t                    hp_per_level{2};
    int8_t                     passive_regen{0};  // HP healed at end of each turn
    bool                       is_boss{false};
    std::vector<AIPatternInfo> patterns;
};

} // namespace battle

#endif // BATTLE_ENEMY_INFO_H
