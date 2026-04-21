#ifndef BATTLE_ENEMY_DATABASE_H
#define BATTLE_ENEMY_DATABASE_H

#include "assets/assets_interface.h"
#include "game/battle/enemy_info.h"
#include <memory>
#include <string>
#include <vector>

namespace battle {

// ─────────────────────────────────────────────────────────────────────────────
//  EnemyDatabase
//   Loads enemy definitions from a Lua manifest at runtime.
//   The Lua state is ephemeral — created, read, and closed inside load().
//   After loading, queries are pure C++ with no Lua overhead.
// ─────────────────────────────────────────────────────────────────────────────

class EnemyDatabase {
  public:
    EnemyDatabase()  = default;
    ~EnemyDatabase() = default;

    // Parse enemies.lua and populate the internal list.
    // Safe to call multiple times (clears previous data).
    void load(const std::string& path, std::shared_ptr<AssetsInterface> assets);

    [[nodiscard]] const EnemyInfo* findByType(EnemyType type) const noexcept;
    [[nodiscard]] const EnemyInfo* findById(const std::string& lua_id) const noexcept;
    [[nodiscard]] bool             isLoaded() const noexcept { return !m_enemies.empty(); }
    [[nodiscard]] int              count()    const noexcept {
        return static_cast<int>(m_enemies.size());
    }

  private:
    std::vector<EnemyInfo> m_enemies;
};

} // namespace battle

#endif // BATTLE_ENEMY_DATABASE_H
