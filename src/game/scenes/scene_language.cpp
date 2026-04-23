#include "game/scenes/scene_language.h"

#include "core/localization.h"
#include "core/window.h"
#include "engine/core/scene_manager.h"
#include "engine/utils/logger.h"

void SceneLanguage::onEnter()
{
    LOG_TRACE("[SceneLanguage] Enter");
    this->m_dispatcher->sink<WindowMouseEvent>().connect<&SceneLanguage::onMouse>(this);

    m_lua = std::make_unique<ScriptManager>();
    m_ui = std::make_unique<UISystem>(m_lua->State(), m_renderer);

    m_ui->AddPanel("bg", {0, 0, 1280, 720}, Color::Black(), 0);

    m_ui->AddLabel("title", {0, 200, 1280, 80}, "Select Language", 48.0f, Color::White(), 1);

    m_ui->AddButton("btn_en", {BTN_EN_X, BTN_Y, BTN_W, BTN_H}, "English", 32.0f, -1,
                    Color(50, 100, 200, 255), 1);

    m_ui->AddButton("btn_th", {BTN_TH_X, BTN_Y, BTN_W, BTN_H}, "ภาษาไทย", 32.0f, -1,
                    Color(40, 160, 80, 255), 1);
}

void SceneLanguage::onUpdate(double deltaTime)
{
    MouseState ms;
    ms.x = m_mouse_x;
    ms.y = m_mouse_y;
    ms.clicked = m_mouse_clicked;
    ms.held = m_mouse_held;
    m_ui->onUpdate(ms);

    if (m_mouse_clicked) {
        if (auto *btn = m_ui->Find("btn_en"); btn && btn->hovered) {
            Localization::setLanguage(Language::English);
            this->m_dispatcher->trigger(SceneEvent{"story"});
        } else if (auto *btn = m_ui->Find("btn_th"); btn && btn->hovered) {
            Localization::setLanguage(Language::Thai);
            this->m_dispatcher->trigger(SceneEvent{"story"});
        }
    }

    m_mouse_clicked = false;
}

void SceneLanguage::onDraw()
{
    this->m_cursor->onDraw();
    m_ui->onDraw();
}

void SceneLanguage::onExit()
{
    LOG_TRACE("[SceneLanguage] Exit");
    this->m_dispatcher->sink<WindowMouseEvent>().disconnect(this);
    m_ui.reset();
    m_lua.reset();
}

void SceneLanguage::onMouse(const WindowMouseEvent &event)
{
    m_mouse_x = float(event.mouseX);
    m_mouse_y = float(event.mouseY);
    if (event.click) {
        if (!m_mouse_held)
            m_mouse_clicked = true;
        m_mouse_held = true;
    } else {
        m_mouse_held = false;
    }
}
