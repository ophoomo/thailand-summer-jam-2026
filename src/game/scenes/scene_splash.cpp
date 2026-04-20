
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
}

void SceneSplash::onUpdate(double deltaTime)
{
    this->splashTimer += deltaTime;
    if (this->splashTimer >= 4.0) {
        // this->m_dispatcher->trigger(SceneEvent{"menu"});
    }
}

void SceneSplash::onDraw()
{

    this->m_renderer->oxDrawRectangle(200, 200, 200, 200, Color::Red());
}

void SceneSplash::onExit()
{
    LOG_TRACE("[SceneSplash] Exit");
}

// ============================================================
// Private Methods
// ============================================================
