
#include "game/battle/enemy_ai.h"
#include "utils/logger.h"
#include <algorithm>
#include <cstdlib>

namespace battle {

// ─────────────────────────────────────────────────────────────────────────────
//  spawn
// ─────────────────────────────────────────────────────────────────────────────

entt::entity EnemyAI::spawn(entt::registry &reg, const EnemyInfo &info, int32_t level,
                            int32_t priority)
{
    int32_t hp = info.base_hp + info.hp_per_level * (level - 1);

    entt::entity e = reg.create();
    reg.emplace<EnemyTag>(e);
    reg.emplace<NameComp>(e, info.name);
    reg.emplace<HealthComp>(e, HealthComp{hp, hp});
    reg.emplace<BlockComp>(e);
    reg.emplace<StrengthComp>(e);
    reg.emplace<StatusComp>(e);
    reg.emplace<TurnOrderComp>(e, TurnOrderComp{priority});
    reg.emplace<EnemyTypeComp>(e, EnemyTypeComp{info.type, level, 0});

    // Add sprite component with the sprite from Lua definition
    reg.emplace<SpriteComp>(e, SpriteComp{
                                   info.sprite_id, // texture_id
                                   800.0f,         // x (right side of screen)
                                   250.0f,         // y (center vertically)
                                   156.81f,        // width (same as player)
                                   226.5f          // height (same as player)
                               });

    if (info.is_boss)
        reg.emplace<BossTag>(e);

    if (info.passive_regen > 0)
        reg.get<StatusComp>(e).regen = info.passive_regen;

    // Pre-compute first intent so the UI can display it before the first turn
    resolveIntent(reg, e, info);

    LOG_TRACE("[EnemyAI] Spawned {} (level={} hp={})", info.name, level, hp);
    return e;
}

// ─────────────────────────────────────────────────────────────────────────────
//  resolveIntent — advance pattern index, write IntentComp from EnemyInfo data
// ─────────────────────────────────────────────────────────────────────────────

void EnemyAI::resolveIntent(entt::registry &reg, entt::entity enemy, const EnemyInfo &info)
{
    if (info.patterns.empty())
        return;

    auto &et = reg.get<EnemyTypeComp>(enemy);
    size_t idx = static_cast<size_t>(et.pattern_index) % info.patterns.size();
    const AIPatternInfo &pat = info.patterns[idx];

    // Scale damage +5% per level above 1
    float scale = 1.0f + 0.05f * static_cast<float>(et.level - 1);
    int32_t scaled_dmg = static_cast<int32_t>(static_cast<float>(pat.damage) * scale);

    reg.emplace_or_replace<IntentComp>(enemy,
                                       IntentComp{pat.action, scaled_dmg, pat.block, pat.times});
}

// ─────────────────────────────────────────────────────────────────────────────
//  executeIntent
// ─────────────────────────────────────────────────────────────────────────────

int32_t EnemyAI::executeIntent(entt::registry &reg, entt::entity enemy, entt::entity player,
                               const EnemyInfo &info)
{
    auto &et = reg.get<EnemyTypeComp>(enemy);
    auto &intent = reg.get<IntentComp>(enemy);
    int32_t raw_damage = 0;

    switch (intent.type) {

    case IntentType::ATTACK:
    case IntentType::SPECIAL:
        // BattleSystem owns the damage pipeline (block absorption, events)
        raw_damage = intent.damage * intent.times;
        break;

    case IntentType::DEFEND:
        reg.get<BlockComp>(enemy).amount += intent.block;
        break;

    case IntentType::BUFF:
        // Passive effects (regen, strength) are handled by TurnManager via StatusComp
        break;

    case IntentType::DEBUFF: {
        if (info.patterns.empty())
            break;
        size_t idx = static_cast<size_t>(et.pattern_index) % info.patterns.size();
        const AIPatternInfo &pat = info.patterns[idx];
        auto &st = reg.get<StatusComp>(player);
        st.vulnerable =
            static_cast<int8_t>(std::clamp<int32_t>(st.vulnerable + pat.apply_vulnerable, 0, 127));
        st.weak = static_cast<int8_t>(std::clamp<int32_t>(st.weak + pat.apply_weak, 0, 127));
        break;
    }
    }

    ++et.pattern_index;
    return raw_damage;
}

} // namespace battle
