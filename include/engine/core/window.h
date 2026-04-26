#ifndef EDBCA38A_F47C_42A9_AA0B_C91ABC745EE2
#define EDBCA38A_F47C_42A9_AA0B_C91ABC745EE2

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "entt/entt.hpp"
#include <memory>
#include <string>
#include <vector>

struct WindowKeyEvent
{
    int key;
};
struct WindowMouseEvent
{
    int mouseX;
    int mouseY;
    Uint8 click;
};

class Window
{
  public:
    Window(const int width, const int height, const std::string &title,
           std::shared_ptr<entt::dispatcher> dispatcher);
    ~Window();

    void pollEvents();
    void getFramebufferSize(int &width, int &height);
    void getWindowResize();
    Uint64 getPerformanceFrequency();
    Uint64 getPerformanceCounter();
    std::vector<const char *> GetInstanceExtensions();
    bool isMinimized() const { return this->m_minimized; }

    SDL_Window *get() const
    {
        return this->m_window.get();
    }
    bool wasResized() const
    {
        return this->m_framebufferResized;
    }
    void resetResizedFlag()
    {
        this->m_framebufferResized = false;
    }
    bool shouldClose() const
    {
        return this->done;
    }
    std::string getTitle() const
    {
        return this->m_title;
    }
    int getWidth() const
    {
        return this->m_width;
    }
    int getHeight() const
    {
        return this->m_height;
    }
    void shutdown()
    {
        this->done = true;
    }

  private:
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_window;
    std::shared_ptr<entt::dispatcher> m_dispatcher;
    std::unique_ptr<SDL_Event> m_event;

    bool done{false};
    std::string m_title;
    int m_width;
    int m_height;
    bool m_framebufferResized{false};
    int m_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    bool m_minimized{false};

};

#endif /* EDBCA38A_F47C_42A9_AA0B_C91ABC745EE2 */
