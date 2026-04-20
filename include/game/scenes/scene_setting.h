#ifndef CA0AF0A3_C593_4F75_B0B4_E63D90B2DF68
#define CA0AF0A3_C593_4F75_B0B4_E63D90B2DF68

#include "engine/core/scene.h"

class SceneSetting : public Scene
{
  public:
    SceneSetting(std::shared_ptr<entt::dispatcher> dispatcher, std::shared_ptr<OxRenderer> renderer,
                 std::shared_ptr<AssetsInterface> assets, std::shared_ptr<AudioInterface> audio)
        : Scene(dispatcher, renderer, assets, audio)
    {
    }
    ~SceneSetting() = default;

    void onEnter() override;
    void onDraw() override;
    void onUpdate(double deltaTime) override;
    void onExit() override;
};

#endif /* CA0AF0A3_C593_4F75_B0B4_E63D90B2DF68 */
