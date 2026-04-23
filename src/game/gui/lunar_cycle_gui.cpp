
#include "game/gui/lunar_cycle_gui.h"
#include "game/battle/components.h"
#include "glm/trigonometric.hpp"
#include "renderer/color.h"
#include "renderer/text_effect.h"
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
                                       Color::White(), 4);
    }
    this->m_renderer->oxDrawSprite(20, 360, LUNAR_CYCLE_WIDTH, LUNAR_CYCLE_HEIGHT, "lunar_compass",
                                   Color::White(), 5, glm::radians(this->m_current_rotate));

    std::string t = std::format("{}/{}", this->m_mana, this->m_max_mana);
    this->m_renderer->oxDrawText(240, 680, t.c_str(), 30, Color::White(),
                                 TextEffect::Outline(Color::Blue()));
}

void LunarCycleGUI::onUpdate(double dt)
{
    float speed = 180.0f;

    float diff = m_target_rotate - m_current_rotate;

    while (diff > 180.0f)
        diff -= 360.0f;
    while (diff < -180.0f)
        diff += 360.0f;

    if (fabs(diff) < 0.1f) {
        m_current_rotate = m_target_rotate;
        return;
    }

    float step = speed * dt;

    if (step > fabs(diff))
        step = fabs(diff);

    m_current_rotate += step * (diff > 0.0f ? 1.0f : -1.0f);

    m_current_rotate = fmod(m_current_rotate, 360.0f);
    if (m_current_rotate < 0.0f)
        m_current_rotate += 360.0f;
}

void LunarCycleGUI::onMana(battle::BattleSystem *m_battle)
{
    auto &reg = m_battle->getRegistry();
    entt::entity player = this->m_battle->getPlayer();
    if (auto *en = reg.try_get<battle::EnergyComp>(player)) {
        this->m_mana = en->current;
        this->m_max_mana = en->max;
        switch (this->m_mana) {
        case 4:
            this->m_target_rotate = LUNAR_CYCLE_MANA_4;
            break;
        case 3:
            this->m_target_rotate = LUNAR_CYCLE_MANA_3;
            break;
        case 2:
            this->m_target_rotate = LUNAR_CYCLE_MANA_2;
            break;
        case 1:
            this->m_target_rotate = LUNAR_CYCLE_MANA_1;
            break;
        default:
            this->m_target_rotate = LUNAR_CYCLE_MANA_0;
        }
    }
}
