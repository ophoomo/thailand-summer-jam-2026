#ifndef B21E8FB9_EE83_4642_9675_594206EBD3B8
#define B21E8FB9_EE83_4642_9675_594206EBD3B8

#include "engine/core/scene.h"
#include "renderer/renderer_interface.h"

class SceneGame : public Scene
{
  public:
    SceneGame(std::shared_ptr<entt::dispatcher> dispatcher, std::shared_ptr<OxRenderer> renderer,
              std::shared_ptr<AssetsInterface> assets, std::shared_ptr<AudioInterface> audio)
        : Scene(dispatcher, renderer, assets, audio)
    {
    }
    ~SceneGame() = default;

    void onEnter() override;
    void onDraw() override;
    void onUpdate(double deltaTime) override;
    void onExit() override;

  private:
    TextureHandle m_gameplay_bg;
};

#endif /* B21E8FB9_EE83_4642_9675_594206EBD3B8 */
