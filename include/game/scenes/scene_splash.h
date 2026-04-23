#ifndef D09019B3_E41F_43AF_B543_9D8C2E095A48
#define D09019B3_E41F_43AF_B543_9D8C2E095A48

#include "engine/core/scene.h"

static constexpr float FADE_IN_TIME  = 1.5f;
static constexpr float HOLD_TIME     = 1.0f;
static constexpr float FADE_OUT_TIME = 1.5f;
static constexpr float TOTAL_TIME    = FADE_IN_TIME + HOLD_TIME + FADE_OUT_TIME;

static constexpr float LOGO_W = 702.0f;
static constexpr float LOGO_H = 144.0f;
static constexpr float LOGO_X = (1280.0f - LOGO_W) / 2.0f;
static constexpr float LOGO_Y = (720.0f  - LOGO_H) / 2.0f;

class SceneSplash : public Scene
{
  public:
    SceneSplash(std::shared_ptr<entt::dispatcher> dispatcher, std::shared_ptr<OxRenderer> renderer,
                std::shared_ptr<AssetsInterface> assets, std::shared_ptr<AudioInterface> audio)
        : Scene(dispatcher, renderer, assets, audio)
    {
    }
    ~SceneSplash() = default;

    void onEnter() override;
    void onDraw() override;
    void onUpdate(double deltaTime) override;
    void onExit() override;

  private:
    double splashTimer = 0;
    float m_alpha = 0.0f;
    bool checkExit = false;
};

#endif /* D09019B3_E41F_43AF_B543_9D8C2E095A48 */
