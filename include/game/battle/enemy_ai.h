#ifndef F91D2940_8893_4519_851E_3D0D3C18C13B
#define F91D2940_8893_4519_851E_3D0D3C18C13B

#include "game/battle/components.h"
#include "game/battle/enemy_info.h"
#include <entt/entt.hpp>

namespace battle {

// ─────────────────────────────────────────────────────────────────────────────
//  EnemyAI — stateless helpers; all state lives in ECS components.
//  Data (HP, patterns) comes from EnemyInfo loaded by EnemyDatabase.
// ─────────────────────────────────────────────────────────────────────────────

class EnemyAI {
  public:
    EnemyAI() = delete;

    // Create a fully initialized enemy entity from a loaded EnemyInfo definition
    static entt::entity spawn(entt::registry& reg, const EnemyInfo& info,
                               int32_t level, int32_t priority);

    // Compute next intent from the info's pattern cycle and write IntentComp
    static void resolveIntent(entt::registry& reg, entt::entity enemy,
                               const EnemyInfo& info);

    // Execute the stored IntentComp against the player.
    // Returns raw damage (0 for non-attack intents).
    // Block absorption and event dispatch are handled by BattleSystem.
    static int32_t executeIntent(entt::registry& reg, entt::entity enemy,
                                  entt::entity player, const EnemyInfo& info);
};

} // namespace battle

#endif // F91D2940_8893_4519_851E_3D0D3C18C13B
