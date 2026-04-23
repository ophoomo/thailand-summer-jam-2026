#ifndef DC5537C7_2DAA_4B34_8489_87B38ACE80D8
#define DC5537C7_2DAA_4B34_8489_87B38ACE80D8

#include "assets/assets_interface.h"
#include "renderer/ox_renderer.h"
#include <memory>

class CursorUI
{
  public:
    CursorUI(std::shared_ptr<entt::dispatcher> m_dispatcher, std::shared_ptr<OxRenderer> OxRenderer,
             std::shared_ptr<AssetsInterface> m_assets);
    ~CursorUI();

    void onEnter();

    void onDraw();

  private:
    void onMouse(const WindowMouseEvent &event);

    std::shared_ptr<entt::dispatcher> m_dispatcher;
    std::shared_ptr<OxRenderer> m_renderer;
    std::shared_ptr<AssetsInterface> m_assets;

    float m_mouse_x, m_mouse_y;
    bool on_click;
    int on_click_timer = 0;
};

#endif /* DC5537C7_2DAA_4B34_8489_87B38ACE80D8 */
