#ifndef D09019B3_E41F_43AF_B543_9D8C2E095A48
#define D09019B3_E41F_43AF_B543_9D8C2E095A48

#include "engine/core/scene.h"

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
};

#endif /* D09019B3_E41F_43AF_B543_9D8C2E095A48 */
