#ifndef HELIORA_SCENE_STORY_H
#define HELIORA_SCENE_STORY_H

#include "engine/core/scene.h"

#include "engine/core/scene.h"
#include "renderer/renderer_interface.h"
#include "scripting/script_manager.h"

class SceneStory : public Scene
{
  public:
    SceneStory(std::shared_ptr<entt::dispatcher> dispatcher, std::shared_ptr<OxRenderer> renderer,
               std::shared_ptr<AssetsInterface> assets, std::shared_ptr<AudioInterface> audio,
               std::shared_ptr<CursorUI> cursor)
        : Scene(dispatcher, renderer, assets, audio, cursor)
    {
    }
    ~SceneStory() = default;

    void onEnter() override;
    void onDraw() override;
    void onUpdate(double deltaTime) override;
    void onExit() override;

  private:
    void onMouse(const WindowMouseEvent &event);

    std::unique_ptr<ScriptManager> m_lua;
    std::unique_ptr<UISystem> m_ui;

    float m_mouse_x = 0;
    float m_mouse_y = 0;
    bool m_mouse_clicked = false;
    bool m_mouse_held = false;
};
#endif // HELIORA_SCENE_STORY_H
