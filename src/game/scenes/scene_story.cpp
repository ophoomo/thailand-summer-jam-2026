#include "game/scenes/scene_story.h"
#include "engine/core/scene_manager.h"
#include "engine/utils/logger.h"

static constexpr const char *STORY_SCRIPT = "assets/scripts/ui/ui_story.lua";

void SceneStory::onEnter()
{
    LOG_TRACE("[SceneStory] Enter");
    this->m_dispatcher->sink<WindowMouseEvent>().connect<&SceneStory::onMouse>(this);

    this->m_lua = std::make_unique<ScriptManager>();
    this->m_ui = std::make_unique<UISystem>(this->m_lua->State(), m_renderer);

    this->m_lua->BindApp(this->m_dispatcher.get());
    this->m_lua->BindScene(this->m_dispatcher.get(), "story");
    this->m_lua->BindUI(m_ui.get());
    this->m_lua->BindAudio(this->m_audio.get());
    this->m_lua->BindInput(&m_mouse_x, &m_mouse_y, &m_mouse_clicked, &m_mouse_held);
    this->m_lua->BindLocalization();

    auto script = this->m_assets->loadText(STORY_SCRIPT);
    if (!m_lua->RunScript(STORY_SCRIPT, script)) {
        LOG_CORE_WARN("[SceneStory] Could not load '{}' — skipping story", STORY_SCRIPT);
    }

    this->m_lua->CallVoid("on_enter");
}

void SceneStory::onUpdate(double deltaTime)
{
    MouseState ms;
    ms.x = m_mouse_x;
    ms.y = m_mouse_y;
    ms.clicked = m_mouse_clicked;
    ms.held = m_mouse_held;

    this->m_ui->onUpdate(ms);
    this->m_lua->CallWithFloat("on_update", float(deltaTime));
    this->m_mouse_clicked = false;
}

void SceneStory::onDraw()
{
    this->m_renderer->oxDrawRectangle(0, 0, 1280, 720, {0, 0, 0, 255});
    this->m_ui->onDraw();
}

void SceneStory::onExit()
{
    LOG_TRACE("[SceneStory] Exit");
    this->m_dispatcher->sink<WindowMouseEvent>().disconnect(this);
    this->m_lua->CallVoid("on_exit");
    this->m_ui.reset();
    this->m_lua.reset();
}

void SceneStory::onMouse(const WindowMouseEvent &event)
{
    this->m_mouse_x = float(event.mouseX);
    this->m_mouse_y = float(event.mouseY);
    if (event.click) {
        if (!this->m_mouse_held)
            this->m_mouse_clicked = true;
        this->m_mouse_held = true;
    } else {
        this->m_mouse_held = false;
    }
}
