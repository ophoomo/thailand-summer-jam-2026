#ifndef D485A3E5_BDFD_4A05_8305_F521F4218C02
#define D485A3E5_BDFD_4A05_8305_F521F4218C02

#include "assets/assets_interface.h"
#include "audio/audio_interface.h"
#include "core/scene_manager.h"
#include "core/window.h"
#include "entt/entt.hpp"
#include "entt/signal/fwd.hpp"
#include "renderer/ox_renderer.h"
#include "ui/debug/debug_tool.h"

#include <memory>
#include <string>

struct ApplicationEvent
{
    bool shuntdown;
};

class Application
{
  public:
    Application(const int width, const int height, const std::string &title);
    ~Application();

    void run();

    SceneManager *getScene() const
    {
        return this->m_scenes.get();
    }
    std::shared_ptr<entt::dispatcher> getDispatcher()
    {
        return this->m_dispatcher;
    }
    std::shared_ptr<OxRenderer> getRenderer()
    {
        return this->m_renderer;
    }
    std::shared_ptr<AssetsInterface> getAssets()
    {
        return this->m_assets;
    }
    std::shared_ptr<AudioInterface> getAudio()
    {
        return this->m_audio;
    }

  private:
    void init();
    void mainLoop();
    void cleanup();
    void loadFont();
    void loadAudio();
    void loadLocalization();
    void onEvent(const ApplicationEvent &event);

    std::shared_ptr<AssetsInterface> m_assets;
    std::shared_ptr<AudioInterface> m_audio;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<OxRenderer> m_renderer;
    std::unique_ptr<SceneManager> m_scenes;
    std::shared_ptr<entt::dispatcher> m_dispatcher;

    std::string m_windowTitle = "Unknow Game Title";
    int m_width = 800;
    int m_height = 600;

    double m_deltaTime;

#ifndef NDEBUG
    std::unique_ptr<DebugTools> m_debug_tools;
#endif
};

#endif /* D485A3E5_BDFD_4A05_8305_F521F4218C02 */
