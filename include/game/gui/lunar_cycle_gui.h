#ifndef DA17FEB0_EEC3_4578_B476_349E78832C66
#define DA17FEB0_EEC3_4578_B476_349E78832C66

#include "assets/assets_interface.h"
#include "audio/audio_interface.h"
#include "renderer/ox_renderer.h"
#include <memory>

static constexpr float LUNAR_CYCLE_WIDTH = 200;
static constexpr float LUNAR_CYCLE_HEIGHT = 200;

class LunarCycleGUI
{
  public:
    LunarCycleGUI(std::shared_ptr<OxRenderer> m_renderer, std::shared_ptr<AssetsInterface> m_assets,
                  std::shared_ptr<AudioInterface> m_audio);
    ~LunarCycleGUI();

    void onEnter();
    void onDraw();
    void onUpdate(double dt);

  private:
    std::shared_ptr<OxRenderer> m_renderer;
    std::shared_ptr<AssetsInterface> m_assets;
    std::shared_ptr<AudioInterface> m_audio;

    float m_rotate = 0.0f;
    float m_timer = 0.0f;
};

#endif /* DA17FEB0_EEC3_4578_B476_349E78832C66 */
