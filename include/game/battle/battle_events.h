#ifndef BATTLE_EVENTS_H
#define BATTLE_EVENTS_H

#include <cstdint>
#include <entt/entt.hpp>

// ─────────────────────────────────────────────────────────────────────────────
//  All events published via entt::dispatcher.
//  Subscribers (UI, audio, animation) connect to these; the battle logic never
//  knows who is listening — pure decoupling.
// ─────────────────────────────────────────────────────────────────────────────

namespace battle {

struct EvTurnStarted {
    entt::entity actor;
    bool         is_player;
};

struct EvTurnEnded {
    entt::entity actor;
    bool         is_player;
};

struct EvCardDrawn {
    entt::entity player;
    int32_t      card_id;
    int32_t      hand_slot;
};

struct EvCardPlayed {
    entt::entity player;
    int32_t      card_id;
    entt::entity target;   // entt::null if no target
};

struct EvCardDiscarded {
    entt::entity player;
    int32_t      card_id;
    int32_t      hand_slot;
};

struct EvDamageDealt {
    entt::entity attacker;
    entt::entity target;
    int32_t      raw_damage;
    int32_t      actual_damage;    // after block absorption
    int32_t      block_absorbed;
};

struct EvBlockGained {
    entt::entity entity;
    int32_t      amount;
};

struct EvStatusApplied {
    entt::entity target;
    int8_t       poison_delta;
    int8_t       vulnerable_delta;
    int8_t       weak_delta;
};

struct EvEntityDied {
    entt::entity entity;
    bool         is_player;
};

struct EvEnemyIntentChanged {
    entt::entity enemy;
};

struct EvLevelComplete {
    int32_t level_completed;
    int32_t next_level;
};

struct EvBossFightBegin {
    int32_t level;
};

struct EvBattleVictory {};
struct EvBattleDefeat  {};

} // namespace battle

#endif // BATTLE_EVENTS_H
