#ifndef DA17FEB0_EEC3_4578_B476_349E78832C66
#define DA17FEB0_EEC3_4578_B476_349E78832C66

#include "assets/assets_interface.h"
#include "audio/audio_interface.h"
#include "game/battle/battle_system.h"
#include "renderer/ox_renderer.h"
#include <memory>

static constexpr float LUNAR_CYCLE_WIDTH = 320.0f;
static constexpr float LUNAR_CYCLE_HEIGHT = 320.0f;

static constexpr float LUNAR_CYCLE_MANA_0 = 218.0f;
static constexpr float LUNAR_CYCLE_MANA_1 = 290.0f;
static constexpr float LUNAR_CYCLE_MANA_2 = 0.0f;
static constexpr float LUNAR_CYCLE_MANA_3 = 70.0f;
static constexpr float LUNAR_CYCLE_MANA_4 = 142.0f;

class LunarCycleGUI
{
  public:
    LunarCycleGUI(std::shared_ptr<OxRenderer> m_renderer, std::shared_ptr<AssetsInterface> m_assets,
                  std::shared_ptr<AudioInterface> m_audio,
                  std::shared_ptr<battle::BattleSystem> m_battle);
    ~LunarCycleGUI();

    void onEnter();
    void onDraw();
    void onUpdate(double dt);
    void onMana(battle::BattleSystem *m_battle);

  private:
    std::shared_ptr<OxRenderer> m_renderer;
    std::shared_ptr<AssetsInterface> m_assets;
    std::shared_ptr<AudioInterface> m_audio;
    std::shared_ptr<battle::BattleSystem> m_battle;

    float m_target_rotate = 0;
    float m_current_rotate = 0;

    int m_mana = 0;
    int m_max_mana = 0;
};

#endif /* DA17FEB0_EEC3_4578_B476_349E78832C66 */
