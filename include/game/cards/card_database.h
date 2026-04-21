#ifndef CE811A94_2711_408A_8158_95041C7D65C6
#define CE811A94_2711_408A_8158_95041C7D65C6

#include "assets/assets_interface.h"
#include "game/cards/card_info.h"
#include <memory>
#include <string>
#include <vector>

// ============================================================
// CardDatabase
//   Loads card definitions from a Lua manifest at runtime.
//   The Lua state is ephemeral — created, read, and closed
//   inside load(); no Lua state persists after loading.
// ============================================================
class CardDatabase
{
  public:
    CardDatabase() = default;
    ~CardDatabase() = default;

    void load(const std::string &path, std::shared_ptr<AssetsInterface> assets);

    [[nodiscard]] const CardInfo *findById(const std::string &lua_id) const;
    [[nodiscard]] const CardInfo *findByIndex(int index) const;
    [[nodiscard]] int count() const
    {
        return static_cast<int>(m_cards.size());
    }

  private:
    std::vector<CardInfo> m_cards;
};

#endif /* CE811A94_2711_408A_8158_95041C7D65C6 */
