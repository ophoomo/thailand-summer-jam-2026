#ifndef F160F90F_10FD_4512_A68F_27827F25A9CB
#define F160F90F_10FD_4512_A68F_27827F25A9CB

#include <cstdint>
#include <entt/entt.hpp>

namespace battle {

// ─────────────────────────────────────────────────────────────────────────────
//  CombatPhase — the state machine driving BattleSystem::update()
// ─────────────────────────────────────────────────────────────────────────────

enum class CombatPhase : uint8_t {
    IDLE = 0,         // no battle active
    SETUP,            // spawning enemies, resetting context
    PLAYER_DRAW,      // draw one card, restore energy, clear block
    PLAYER_TURN,      // waiting for player input
    ENEMY_TURN,       // enemies execute their intents one by one
    RESOLVE_DEATHS,   // remove dead entities, check win/loss conditions
    LEVEL_TRANSITION, // brief pause before next encounter
    BOSS_INTRO,       // pause for boss cutscene / fanfare
    VICTORY,          // all enemies dead
    DEFEAT,           // player HP <= 0
};

// ─────────────────────────────────────────────────────────────────────────────
//  CombatContext — mutable runtime state shared across all systems
// ─────────────────────────────────────────────────────────────────────────────

struct CombatContext {
    CombatPhase  phase{CombatPhase::IDLE};

    entt::entity player_entity{entt::null};

    int32_t turn_number{0};          // increments every full player+enemy cycle
    int32_t enemies_alive{0};        // decremented when DeadTag entities are removed
    int32_t enemy_turn_index{0};     // which enemy is currently acting

    float   phase_timer{0.0f};       // seconds spent in current phase
    bool    player_ended_turn{false};

    // Set by BattleSystem when an enemy action needs animation time before
    // the next enemy can act.
    bool    waiting_for_anim{false};
};

} // namespace battle

#endif // F160F90F_10FD_4512_A68F_27827F25A9CB
