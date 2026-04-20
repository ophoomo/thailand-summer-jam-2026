
#include "core/window.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"
#include "imgui_impl_sdl3.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

Window::Window(const int width, const int height, const std::string &title,
               std::shared_ptr<entt::dispatcher> dispatcher)
    : m_window(nullptr, SDL_DestroyWindow)
{
    LOG_CORE_INFO("[Window] Initialize");

    this->m_dispatcher = dispatcher;
    this->m_title = title;
    this->m_width = width;
    this->m_height = height;

    if (!SDL_Init(this->m_flags)) {
        auto error = std::format("Error initialize SDL3: {}", SDL_GetError());
        LOG_CORE_ERROR(error);
        throw std::runtime_error(error);
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags =
        SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    this->m_window.reset(
        SDL_CreateWindow(this->m_title.c_str(), this->m_width, this->m_height, window_flags));
    if (!this->m_window) {
        auto error = std::format("Error creating Window: {}", SDL_GetError());
        LOG_CORE_ERROR(error);
        throw std::runtime_error(error);
    }
    LOG_CORE_TRACE("[Window] Create Window Success");

    if (!SDL_Vulkan_LoadLibrary(nullptr)) {
        auto error = std::format("Error SDL Not Support Vulkan");
        LOG_CORE_ERROR(error);
        throw std::runtime_error(error);
    }
    LOG_CORE_TRACE("[Window] Support Vulkan");

    this->m_event = std::make_unique<SDL_Event>();

    SDL_SetWindowPosition(this->m_window.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(this->m_window.get());
}

Window::~Window()
{
    LOG_CORE_INFO("[Window] Cleanup");
    this->m_window.reset();
    SDL_Quit();
}

// ============================================================
// Public Methods
// ============================================================

void Window::pollEvents()
{
    while (SDL_PollEvent(m_event.get())) {
#ifndef NDEBUG
        ImGui_ImplSDL3_ProcessEvent(m_event.get());
        const ImGuiIO &imgui_io = ImGui::GetIO();
#endif
        switch (m_event->type) {
        case SDL_EVENT_QUIT:
            done = true;
            break;
        case SDL_EVENT_KEY_DOWN:
#ifndef NDEBUG
            if (imgui_io.WantCaptureKeyboard)
                break;
#endif
            m_dispatcher->trigger(WindowKeyEvent{m_event->key.scancode});
            break;
        case SDL_EVENT_MOUSE_MOTION:
#ifndef NDEBUG
            if (imgui_io.WantCaptureMouse)
                break;
#endif
            m_dispatcher->trigger(WindowMouseEvent{
                static_cast<int>(m_event->motion.x),
                static_cast<int>(m_event->motion.y),
                0,
            });
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
#ifndef NDEBUG
            if (imgui_io.WantCaptureMouse)
                break;
#endif
            m_dispatcher->trigger(WindowMouseEvent{
                static_cast<int>(m_event->button.x),
                static_cast<int>(m_event->button.y),
                m_event->button.button,
            });
            break;
        }
    }
}

std::vector<const char *> Window::GetInstanceExtensions()
{
    Uint32 count;
    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&count);

    return std::vector<const char *>(extensions, extensions + count);
}

void Window::getWindowResize()
{
    int width, height;
    SDL_GetWindowSize(this->m_window.get(), &width, &height);
    if (width != this->m_width || height != this->m_height) {
        this->m_width = width;
        this->m_height = height;
        this->m_framebufferResized = true;
    }
}

Uint64 Window::getPerformanceFrequency()
{
    return SDL_GetPerformanceFrequency();
}

Uint64 Window::getPerformanceCounter()
{
    return SDL_GetPerformanceCounter();
}
