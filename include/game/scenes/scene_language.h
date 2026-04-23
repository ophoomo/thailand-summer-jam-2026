#ifndef TSJ2026_SCENE_LANGUAGE_H
#define TSJ2026_SCENE_LANGUAGE_H

#include "engine/core/scene.h"
#include "scripting/script_manager.h"
#include "ui/ui_system.h"

static constexpr float BTN_W = 300.0f;
static constexpr float BTN_H = 80.0f;
static constexpr float BTN_Y = (720.0f - BTN_H) / 2.0f;
static constexpr float BTN_EN_X = (1280.0f / 2.0f) - BTN_W - 40.0f;
static constexpr float BTN_TH_X = (1280.0f / 2.0f) + 40.0f;

class SceneLanguage : public Scene
{
  public:
    SceneLanguage(std::shared_ptr<entt::dispatcher> dispatcher,
                  std::shared_ptr<OxRenderer> renderer, std::shared_ptr<AssetsInterface> assets,
                  std::shared_ptr<AudioInterface> audio, std::shared_ptr<CursorUI> cursor)
        : Scene(dispatcher, renderer, assets, audio, cursor)
    {
    }
    ~SceneLanguage() = default;

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

#endif // TSJ2026_SCENE_LANGUAGE_H
