#ifndef B93435D3_95C8_4D99_B58B_6FEECC15554E
#define B93435D3_95C8_4D99_B58B_6FEECC15554E

#include "game/battle/combat_state.h"
#include "game/battle/battle_events.h"
#include "game/battle/components.h"
#include <entt/entt.hpp>
#include <vector>

namespace battle {

// ─────────────────────────────────────────────────────────────────────────────
//  TurnManager — structural turn bookkeeping only.
//  All combat-logic calls that need EnemyInfo (resolveIntent, executeIntent)
//  are handled by BattleSystem, which owns the EnemyDatabase.
// ─────────────────────────────────────────────────────────────────────────────

class TurnManager {
  public:
    explicit TurnManager(entt::dispatcher& dispatcher) noexcept;

    // ── Player turn ──────────────────────────────────────────────────────────

    // Restore energy, clear block, tick start-of-turn statuses, draw 1 card
    void beginPlayerTurn(entt::registry& reg, CombatContext& ctx);

    // Discard hand, tick end-of-turn statuses
    void endPlayerTurn(entt::registry& reg, CombatContext& ctx);

    // ── Enemy turn ordering ───────────────────────────────────────────────────

    // Sort living enemies by TurnOrderComp::priority — call once per enemy phase
    void buildEnemyOrder(entt::registry& reg);

    // Apply start-of-turn effects for the current enemy, fire EvTurnStarted.
    // Returns the entity that should act, or entt::null if the turn is skipped
    // (e.g., the enemy died to poison before its turn).
    entt::entity beginEnemyTurn(entt::registry& reg, CombatContext& ctx);

    // Apply end-of-turn effects, fire EvTurnEnded, advance ctx.enemy_turn_index
    void endEnemyTurn(entt::registry& reg, CombatContext& ctx, entt::entity enemy);

    // Called after all enemies have acted — clears the order list
    void endEnemyTurns(entt::registry& reg);

    [[nodiscard]] bool hasMoreEnemies(const CombatContext& ctx) const noexcept {
        return ctx.enemy_turn_index < static_cast<int32_t>(m_enemy_order.size());
    }

    // ── Card draw ────────────────────────────────────────────────────────────

    // Draw one card; reshuffles discard if draw pile is empty.
    // Returns card_id drawn, or -1 if deck and discard are both empty.
    int32_t drawCard(entt::registry& reg, entt::entity player);

  private:
    void applyStartOfTurn(entt::registry& reg, entt::entity entity);
    void applyEndOfTurn(entt::registry& reg, entt::entity entity);

    entt::dispatcher&         m_dispatcher;
    std::vector<entt::entity> m_enemy_order;
};

} // namespace battle

#endif // B93435D3_95C8_4D99_B58B_6FEECC15554E
