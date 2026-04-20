#ifndef A9E6FCC3_8D7F_472D_AB52_719AB904FD43
#define A9E6FCC3_8D7F_472D_AB52_719AB904FD43

#include "core/scene.h"
#include "core/window.h"
#include "entt/signal/fwd.hpp"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

struct SceneEvent
{
    const std::string scene;
};
enum class TransitionState { None, FadeOut, FadeIn };

class SceneManager
{
  public:
    SceneManager(std::shared_ptr<entt::dispatcher> dispatcher, std::shared_ptr<OxRenderer> renderer,
                 std::shared_ptr<Window> window);
    ~SceneManager();

    void onUpdate(double deltaTime);
    void onDraw();
    void onEvent();

    void addScene(std::shared_ptr<Scene> scene, const std::string name);
    void change(const std::string name);

    const std::string &CurrentName() const
    {
        return current;
    }
    const std::string &PreviousName() const
    {
        return previous;
    }
    std::vector<std::string> SceneNames() const;

  private:
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
    std::shared_ptr<entt::dispatcher> m_dispatcher;
    std::shared_ptr<Scene> currentScene;
    std::shared_ptr<OxRenderer> m_renderer;
    std::shared_ptr<Window> m_window;

    std::optional<std::string> m_pendingScene;
    std::optional<std::string> next;
    std::string current;
    std::string previous;

    TransitionState m_transState = TransitionState::None;
    float m_fadeAlpha = 0.0f;
    float m_fadeDuration = 0.4f;

    void onChangeScene();
    void onSceneEvent(const SceneEvent &event);
};

#endif /* A9E6FCC3_8D7F_472D_AB52_719AB904FD43 */
