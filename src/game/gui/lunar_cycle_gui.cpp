
#include "game/gui/lunar_cycle_gui.h"
#include "game/battle/components.h"
#include "glm/trigonometric.hpp"
#include "renderer/color.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

LunarCycleGUI::LunarCycleGUI(std::shared_ptr<OxRenderer> m_renderer,
                             std::shared_ptr<AssetsInterface> m_assets,
                             std::shared_ptr<AudioInterface> m_audio,
                             std::shared_ptr<battle::BattleSystem> m_battle)
{
    LOG_TRACE("[LunarCycleGUI] Initializing");
    this->m_audio = m_audio;
    this->m_assets = m_assets;
    this->m_renderer = m_renderer;
    this->m_battle = m_battle;
}

LunarCycleGUI::~LunarCycleGUI()
{
    LOG_TRACE("[LunarCycleGUI] Destroy");
    for (int i = 0; i < 11; i++) {
        std::string name = std::format("lunar_clock_{}", i);
        this->m_renderer->freeTexture(name);
    }
    this->m_renderer->freeTexture("lunar_compass");
}

// ============================================================
// Public Methods
// ============================================================

void LunarCycleGUI::onEnter()
{
    int width, height, channels;

    for (int i = 0; i < 11; i++) {
        std::string path = std::format("assets/images/gui/lunar_clock_{}.png", i);
        std::string name = std::format("lunar_clock_{}", i);
        auto pixel = this->m_assets->loadImage(path.c_str(), width, height, channels);
        this->m_renderer->createTexture(name, pixel, width, height);
        this->m_assets->unLoadImage(pixel);
    }

    auto pixel =
        this->m_assets->loadImage("assets/images/gui/lunar_compass.png", width, height, channels);
    this->m_renderer->createTexture("lunar_compass", pixel, width, height);
    this->m_assets->unLoadImage(pixel);
}

void LunarCycleGUI::onDraw()
{
    auto &reg = this->m_battle->getRegistry();
    entt::entity player = this->m_battle->getPlayer();

    if (auto *hp = reg.try_get<battle::HealthComp>(player)) {
        float ratio = hp->ratio();
        int index = static_cast<int>(ratio * 10.0f + 0.5f);
        if (index > 10)
            index = 10;
        if (index < 0)
            index = 0;
        std::string name_texture = std::format("lunar_clock_{}", index);
        this->m_renderer->oxDrawSprite(20, 360, LUNAR_CYCLE_WIDTH, LUNAR_CYCLE_HEIGHT, name_texture,
                                       Color::White(), 50);
    }
    this->m_renderer->oxDrawSprite(20, 360, LUNAR_CYCLE_WIDTH, LUNAR_CYCLE_HEIGHT, "lunar_compass",
                                   Color::White(), 3, glm::radians(this->m_rotate), 51);
}

void LunarCycleGUI::onUpdate(double dt)
{
    this->m_timer += dt;
    if (this->m_timer >= 1) {
        this->m_timer = 0;
        this->m_rotate += 45;
    }
    if (this->m_rotate >= 360) {
        this->m_rotate = 0;
    }
}
