#ifndef A5B90A96_0086_4800_B7AF_E0D6A36E3119
#define A5B90A96_0086_4800_B7AF_E0D6A36E3119

#include "engine/core/scene.h"
#include "game/particles/firefly_particles.h"
#include "renderer/renderer_interface.h"
#include "scripting/script_manager.h"

class SceneMenu : public Scene
{
  public:
    SceneMenu(std::shared_ptr<entt::dispatcher> dispatcher, std::shared_ptr<OxRenderer> renderer,
              std::shared_ptr<AssetsInterface> assets, std::shared_ptr<AudioInterface> audio,
              std::shared_ptr<CursorUI> cursor)
        : Scene(dispatcher, renderer, assets, audio, cursor)
    {
    }
    ~SceneMenu() = default;

    void onEnter() override;
    void onDraw() override;
    void onUpdate(double deltaTime) override;
    void onExit() override;

  private:
    void onMouse(const WindowMouseEvent &event);
    void onKeyboard(const WindowKeyEvent &event);

    std::unique_ptr<ScriptManager> m_lua;
    std::unique_ptr<UISystem> m_ui;
    std::unique_ptr<FireflyParticleEmitter> m_mainmenu_particle;

    float m_mouse_x = 0;
    float m_mouse_y = 0;
    bool m_mouse_clicked = false; // true only on the press frame
    bool m_mouse_held = false;    // true while held
};

#endif /* A5B90A96_0086_4800_B7AF_E0D6A36E3119 */
