#ifndef C0BD5DC0_DE26_452E_A27B_C46E20E1A8D0
#define C0BD5DC0_DE26_452E_A27B_C46E20E1A8D0

#include "assets/assets_interface.h"
#include "audio/audio_interface.h"
#include "game/battle/battle_events.h"
#include "game/battle/combat_state.h"
#include "game/battle/components.h"
#include "game/battle/enemy_ai.h"
#include "game/battle/enemy_database.h"
#include "game/battle/turn_manager.h"
#include "game/cards/card_database.h"
#include "game/cards/card_info.h"
#include <entt/entt.hpp>
#include <memory>

namespace battle {

// Returns true when the card only affects the player (no damage / enemy debuff).
inline bool cardTargetsSelf(const CardInfo &card) noexcept
{
    return card.damage == 0 && card.apply_vulnerable == 0 && card.apply_weak == 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  BattleSystem
// ─────────────────────────────────────────────────────────────────────────────

class BattleSystem
{
  public:
    BattleSystem(entt::dispatcher &dispatcher, std::shared_ptr<AssetsInterface> assets,
                 std::shared_ptr<AudioInterface> audio);
    ~BattleSystem() = default;

    BattleSystem(const BattleSystem &) = delete;
    BattleSystem &operator=(const BattleSystem &) = delete;

    // ── Lifecycle ────────────────────────────────────────────────────────────

    void startBattle(int32_t level = 1);
    void update(double dt);
    void shutdown();

    // ── Player input (only processed while phase == PLAYER_TURN) ─────────────

    // preferred_target: use this enemy if alive; falls back to first enemy otherwise
    void playerPlayCard(int32_t hand_slot, entt::entity preferred_target = entt::null);
    void playerEndTurn();
    void playerDiscard(int32_t hand_slot);

    // ── Queries ───────────────────────────────────────────────────────────────

    [[nodiscard]] const CombatContext &getContext() const noexcept
    {
        return m_ctx;
    }
    [[nodiscard]] entt::registry &getRegistry() noexcept
    {
        return m_reg;
    }
    [[nodiscard]] entt::entity getPlayer() const noexcept
    {
        return m_player;
    }
    [[nodiscard]] const CardDatabase &getCardDb() const noexcept
    {
        return m_card_db;
    }
    [[nodiscard]] bool isActive() const noexcept;

  private:
    // ── Setup ────────────────────────────────────────────────────────────────
    void ensureDbLoaded();
    void setupPlayer();
    void spawnEnemies(int32_t level);
    void buildStarterDeck(DeckComp &deck) const;

    // ── Phase machine ────────────────────────────────────────────────────────
    void transitionTo(CombatPhase next);
    void updatePhase(double dt);

    void phasePlayerDraw();
    void phasePlayerTurn(double dt);
    void phaseEnemyTurn(double dt);
    void phaseResolveDeaths();
    void phaseVictory();
    void phaseDefeat();
    void phaseLevelTransition(double dt);

    // ── Combat math ───────────────────────────────────────────────────────────
    [[nodiscard]] int32_t calcDamage(entt::entity attacker, int32_t base,
                                     entt::entity target) const noexcept;
    void applyDamage(entt::entity target, int32_t amount, entt::entity source);
    void applyBlock(entt::entity target, int32_t amount);
    void applyCardEffect(const CardInfo &card, entt::entity target);

    // ── Helpers ───────────────────────────────────────────────────────────────
    // Returns EnemyInfo for an entity, or nullptr if db not loaded / type unknown
    [[nodiscard]] const EnemyInfo *infoFor(entt::entity enemy) const noexcept;

    // ── Data ──────────────────────────────────────────────────────────────────
    // Declaration order matters: m_enemy_db must come before m_turn
    EnemyDatabase m_enemy_db;
    CardDatabase m_card_db;
    entt::registry m_reg;
    entt::dispatcher &m_dispatcher;
    std::shared_ptr<AssetsInterface> m_assets;
    CombatContext m_ctx{};
    TurnManager m_turn;
    entt::entity m_player{entt::null};
    bool m_resolving_card{false};

    static constexpr float ENEMY_ACT_DELAY = 0.9f;
    static constexpr float LEVEL_TRANSITION_DELAY = 2.0f;
    static constexpr float BOSS_INTRO_DELAY = 2.5f;
};

} // namespace battle

#endif // C0BD5DC0_DE26_452E_A27B_C46E20E1A8D0
