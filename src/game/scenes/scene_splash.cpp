
#include "game/scenes/scene_splash.h"
#include "engine/core/scene_manager.h"
#include "engine/utils/logger.h"

// ============================================================
// Public Methods
// ============================================================

void SceneSplash::onEnter()
{
    LOG_TRACE("[SceneSplash] Enter");
    this->splashTimer = 0;
    this->m_alpha = 0.0f;
    this->checkExit = false;

    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/images/logo.png", w, h, c);
    this->m_renderer->createTexture("logo", pixel, w, h);
}

void SceneSplash::onUpdate(double deltaTime)
{
    this->splashTimer += deltaTime;
    float t = (float)this->splashTimer;

    if (t < FADE_IN_TIME) {
        this->m_alpha = t / FADE_IN_TIME;
    } else if (t < FADE_IN_TIME + HOLD_TIME) {
        this->m_alpha = 1.0f;
    } else if (t < TOTAL_TIME) {
        float fadeProgress = (t - FADE_IN_TIME - HOLD_TIME) / FADE_OUT_TIME;
        this->m_alpha = 1.0f - fadeProgress;
    } else {
        if (!this->checkExit) {
            this->m_dispatcher->trigger(SceneEvent{"lang"});
            this->checkExit = true;
        }
    }
}

void SceneSplash::onDraw()
{
    Color tint = Color::Lerp(Color::Transparent(), Color::White(), this->m_alpha);
    this->m_renderer->oxDrawSprite(LOGO_X, LOGO_Y, LOGO_W, LOGO_H, "logo", tint, 0);
}

void SceneSplash::onExit()
{
    LOG_TRACE("[SceneSplash] Exit");
    this->m_renderer->freeTexture("logo");
}

// ============================================================
// Private Methods
// ============================================================
