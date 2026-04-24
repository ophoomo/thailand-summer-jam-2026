#ifndef FB87CE7A_E352_4AC4_A5EE_4A36F873416C
#define FB87CE7A_E352_4AC4_A5EE_4A36F873416C

#include "engine/core/scene.h"

class SceneCredit : public Scene
{
  public:
    SceneCredit(std::shared_ptr<entt::dispatcher> dispatcher, std::shared_ptr<OxRenderer> renderer,
                std::shared_ptr<AssetsInterface> assets, std::shared_ptr<AudioInterface> audio,
                std::shared_ptr<CursorUI> cursor)
        : Scene(dispatcher, renderer, assets, audio, cursor)
    {
    }
    ~SceneCredit() = default;

    void onEnter() override;
    void onDraw() override;
    void onUpdate(double deltaTime) override;
    void onExit() override;
};

#endif /* FB87CE7A_E352_4AC4_A5EE_4A36F873416C */
