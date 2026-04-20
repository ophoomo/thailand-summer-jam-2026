
#include "game/scenes/scene_credit.h"
#include "engine/utils/logger.h"

// ============================================================
// Public Methods
// ============================================================

void SceneCredit::onEnter()
{
    LOG_TRACE("[SceneCredit] Enter");
}

void SceneCredit::onUpdate(double deltaTime) {}

void SceneCredit::onDraw()
{
    this->m_renderer->oxDrawRectangle(200, 200, 200, 200);
}

void SceneCredit::onExit()
{
    LOG_TRACE("[SceneCredit] Exit");
}

// ============================================================
// Private Methods
// ============================================================
