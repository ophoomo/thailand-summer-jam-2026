
#include "game/scenes/scene_game.h"
#include "core/localization.h"
#include "engine/utils/logger.h"
#include "renderer/color.h"
#include "renderer/text_effect.h"

// ============================================================
// Public Methods
// ============================================================

void SceneGame::onEnter()
{
    LOG_TRACE("[SceneGame] Enter");

    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/images/gameplay_bg.png", w, h, c);
    this->m_renderer->createTexture("gameplay_bg", pixel, w, h);

    int channels, sample_rate;
    short *data;
    int sim =
        this->m_assets->loadAudio("assets/audio/gameplay_audio.ogg", channels, sample_rate, data);
    this->m_audio->load("gameplay", channels, sim, sample_rate, data);
    this->m_audio->set_bgm_fade_gain(0);
    this->m_audio->fade_bgm(1.0f, 5.0f);
    this->m_audio->play_bgm("gameplay", true, 0.3);
}

void SceneGame::onUpdate(double deltaTime) {}

void SceneGame::onDraw()
{
    this->m_renderer->oxDrawSprite(0, 0, 1280, 720, "gameplay_bg", Color::White());
    this->m_renderer->oxDrawText(300, 300, Localization::get("game.restart").c_str(), 60,
                                 Color::White(), TextEffect::Glow(Color::Green(), 0.2f), 1);

    this->m_renderer->oxDrawText(600, 300, "สระ อิ อี อุา อู้ ป๊ แบื่น แบ็บก", 60, Color::White(),
                                 TextEffect::Outline(Color::Red(), 0.2f), 1);
    this->m_renderer->oxDrawCircle(200, 200, 50, Color::Red(), 1);
}

void SceneGame::onExit()
{
    LOG_TRACE("[SceneGame] Exit");
    this->m_audio->stop_bgm();
    this->m_renderer->freeTexture("gameplay_bg");
}

// ============================================================
// Private Methods
// ============================================================
