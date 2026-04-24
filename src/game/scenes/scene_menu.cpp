
#include "game/scenes/scene_menu.h"
#include "core/window.h"
#include "engine/utils/logger.h"
#include "game/particles/firefly_particles.h"
#include "scripting/script_manager.h"
#include <memory>

// ============================================================
// Public Methods
// ============================================================

static constexpr const char *MENU_SCRIPT = "assets/scripts/ui/ui_menu.lua";

void SceneMenu::onEnter()
{
    LOG_TRACE("[SceneMenu] Enter");
    this->m_dispatcher->sink<WindowMouseEvent>().connect<&SceneMenu::onMouse>(this);

    this->m_lua = std::make_unique<ScriptManager>();
    this->m_ui = std::make_unique<UISystem>(this->m_lua->State(), m_renderer);
    this->m_mainmenu_particle = std::make_unique<FireflyParticleEmitter>();

    this->m_lua->BindApp(this->m_dispatcher.get());
    this->m_lua->BindScene(this->m_dispatcher.get(), "menu");
    this->m_lua->BindUI(m_ui.get());
    this->m_lua->BindAudio(this->m_audio.get());
    this->m_lua->BindInput(&m_mouse_x, &m_mouse_y, &m_mouse_clicked, &m_mouse_held);
    this->m_lua->BindLocalization();

    auto script = this->m_assets.get()->loadText(MENU_SCRIPT);
    if (!m_lua->RunScript(MENU_SCRIPT, script)) {
        LOG_CORE_WARN("[SceneMenu] Could not load '{}' — "
                      "menu will run without Lua UI",
                      MENU_SCRIPT);
    }

    int channels, sample_rate;
    short *data;
    int sample =
        this->m_assets->loadAudio("assets/audio/menu_bgm.ogg", channels, sample_rate, data);
    this->m_audio->load("menu", channels, sample, sample_rate, data);

    this->m_audio->set_bgm_fade_gain(0);
    this->m_audio->fade_bgm(1.0f, 5.0f);
    this->m_audio->play_bgm("menu", true, 0.3);

    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/images/menu_bg.png", w, h, c);
    this->m_renderer->createTexture("menu_bg", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    this->m_lua->CallVoid("on_enter");
}

void SceneMenu::onUpdate(double deltaTime)
{
    MouseState ms;
    ms.x = m_mouse_x;
    ms.y = m_mouse_y;
    ms.clicked = this->m_mouse_clicked;
    ms.held = this->m_mouse_held;

    this->m_ui->onUpdate(ms);

    this->m_mouse_clicked = false;
    this->m_mainmenu_particle->update(deltaTime, 1280, 720, true);
}

void SceneMenu::onDraw()
{
    this->m_cursor->onDraw();
    this->m_renderer->oxDrawSprite(0, 0, 1280, 720, "menu_bg", {255, 255, 255, 255}, 0);
    this->m_ui->onDraw();
    this->m_mainmenu_particle->draw(this->m_renderer, 1);
}

void SceneMenu::onExit()
{
    LOG_TRACE("[SceneMenu] Exit");
    this->m_dispatcher->sink<WindowMouseEvent>().disconnect(this);
    this->m_ui.reset();
    this->m_lua.reset();
    this->m_audio->stop_bgm();
    this->m_renderer->freeTexture("menu_bg");
    this->m_audio->unload("menu_bg");
}

// ============================================================
// Private Methods
// ============================================================

void SceneMenu::onMouse(const WindowMouseEvent &event)
{
    this->m_mouse_x = float(event.mouseX);
    this->m_mouse_y = float(event.mouseY);
    if (event.click) {
        if (!this->m_mouse_held) {
            this->m_mouse_clicked = true;
        }
        this->m_mouse_held = true;
    } else {
        this->m_mouse_held = false;
    }
}
