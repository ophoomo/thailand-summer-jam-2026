
#include "game/scenes/scene_setting.h"
#include "engine/utils/logger.h"

// ============================================================
// Public Methods
// ============================================================

void SceneSetting::onEnter()
{
    LOG_TRACE("[SceneSetting] Enter");
}

void SceneSetting::onUpdate(double deltaTime) {}

void SceneSetting::onDraw()
{
    this->m_renderer->oxDrawRectangle(200, 200, 200, 200);
}

void SceneSetting::onExit()
{
    LOG_TRACE("[SceneSetting] Exit");
}

// ============================================================
// Private Methods
// ============================================================
