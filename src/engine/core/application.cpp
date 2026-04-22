
#include "core/application.h"
#include "assets/raw/raw_assets.h"
#include "audio/openal/openal_audio.h"
#include "core/localization.h"
#include "core/scene_manager.h"
#include "core/window.h"
#include "entt/signal/fwd.hpp"
#include "renderer/ox_renderer.h"
#include "renderer/renderer_interface.h"
#include "ui/debug/debug_tool.h"
#include "utils/logger.h"
#include "utils/resource_path.h"
#include <memory>
#include <string>

// ============================================================
// Construction / destruction
// ============================================================

Application::Application(const int width, const int height, const std::string &title)
{
    LOG_CORE_INFO("[Application] Initialize");
    this->m_width = width;
    this->m_height = height;
    this->m_windowTitle = title;

    this->init();
}

Application::~Application()
{
    LOG_CORE_INFO("[Application] Cleanup");
    this->cleanup();
}

// ============================================================
// Public Methods
// ============================================================

void Application::run()
{
    this->mainLoop();
}

// ============================================================
// Private Methods
// ============================================================

void Application::init()
{
#ifndef NDEBUG
    this->m_assets = std::make_shared<RawAssets>();
#else
    this->m_assets = std::make_shared<RawAssets>();
    // this->m_assets = std::make_shared<OxenAssests>();
#endif
    this->m_dispatcher = std::make_shared<entt::dispatcher>();
    this->m_window = std::make_shared<Window>(this->m_width, this->m_height, this->m_windowTitle,
                                              this->m_dispatcher);
    ResourcePath::init();
    this->m_renderer = std::make_shared<OxRenderer>(RendererAPI::Vulkan, this->m_window);
    this->m_scenes =
        std::make_unique<SceneManager>(this->m_dispatcher, this->m_renderer, this->m_window);
    this->m_audio = std::make_shared<OpenALAudio>();
    this->m_scenes->onEvent();

#ifndef NDEBUG
    this->m_debug_tools = std::make_unique<DebugTools>(this->m_renderer.get(), this->m_scenes.get(),
                                                       this->m_audio.get());
#endif

    this->m_dispatcher->sink<ApplicationEvent>().connect<&Application::onEvent>(this);
    this->loadFont();
    this->loadAudio();
    this->loadLocalization();
}

void Application::mainLoop()
{
    Uint64 lastCounter = this->m_window->getPerformanceCounter();
    while (!this->m_window->shouldClose()) {
        Uint64 nowCounter = this->m_window->getPerformanceCounter();
        this->m_deltaTime =
            (double)(nowCounter - lastCounter) / this->m_window->getPerformanceFrequency();
        lastCounter = nowCounter;

        this->m_window->pollEvents();

        this->m_window->getWindowResize();
        if (this->m_window->wasResized()) {
            this->m_renderer.get()->resize();
        }

#ifndef NDEBUG
        // this->m_debug_tools->onDraw(this->m_deltaTime);
#endif
        this->m_audio->update(this->m_deltaTime);
        this->m_scenes->onUpdate(this->m_deltaTime);
        this->m_renderer->oxBegin();
        this->m_scenes->onDraw();
        this->m_renderer->oxEnd();
    }
}

void Application::cleanup()
{
    LOG_CORE_INFO("[Application] Cleanup successful");
}

void Application::onEvent(const ApplicationEvent &event)
{
    if (event.shuntdown) {
        this->m_window->shutdown();
    }
}

void Application::loadFont()
{
    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/font/trirong.png", w, h, c);
    auto j = this->m_assets->loadJson("assets/font/trirong.json");
    this->m_renderer->loadFont(j, pixel, w, h);
    this->m_assets->unLoadImage(pixel);
}

void Application::loadAudio()
{
    int channels, sample_rate;
    short *data;
    int sample =
        this->m_assets->loadAudio("assets/audio/click_sfx.ogg", channels, sample_rate, data);
    this->m_audio->load("click", channels, sample, sample_rate, data);
}

void Application::loadLocalization()
{
    auto th = this->m_assets->loadJson("assets/lang/th.json");
    auto en = this->m_assets->loadJson("assets/lang/en.json");
    Localization::load(Language::Thai, th);
    Localization::load(Language::English, en);
}
