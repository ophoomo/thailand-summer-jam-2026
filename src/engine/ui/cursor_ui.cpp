
#include "ui/cursor_ui.h"
#include "utils/logger.h"

CursorUI::CursorUI(std::shared_ptr<entt::dispatcher> m_dispatcher,
                   std::shared_ptr<OxRenderer> m_renderer,
                   std::shared_ptr<AssetsInterface> m_assets)
{
    this->m_assets = m_assets;
    this->m_dispatcher = m_dispatcher;
    this->m_renderer = m_renderer;
}

CursorUI::~CursorUI()
{
    this->m_dispatcher->sink<WindowMouseEvent>().disconnect(this);
    this->m_renderer->freeTexture("cursor");
}

void CursorUI::onEnter()
{
    this->m_dispatcher->sink<WindowMouseEvent>().connect<&CursorUI::onMouse>(this);

    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/images/cursor.png", w, h, c);
    this->m_renderer->createTexture("cursor", pixel, w, h);
}

void CursorUI::onDraw()
{
    Color color = this->on_click ? Color{200, 200, 200, 255} : Color::White();
    this->m_renderer->oxDrawSprite(this->m_mouse_x, this->m_mouse_y, 32, 32, "cursor", color, 1000);
    if (this->on_click) {
        this->on_click_timer += 1;
        if (this->on_click_timer >= 300) {
            this->on_click = false;
            this->on_click_timer = 0;
        }
    }
}

void CursorUI::onMouse(const WindowMouseEvent &event)
{
    this->m_mouse_x = float(event.mouseX);
    this->m_mouse_y = float(event.mouseY);
    if (event.click > 0) {
        this->on_click = true;
    }
}
