
#include "game/gui/timer_gui.h"
#include "assets/assets_interface.h"
#include "renderer/color.h"
#include "renderer/text_effect.h"
#include "utils/logger.h"
#include <string>

// ============================================================
// Construction / destruction
// ============================================================

TimerGUI::TimerGUI(std::shared_ptr<OxRenderer> m_renderer,
                   std::shared_ptr<AssetsInterface> m_assets,
                   std::shared_ptr<AudioInterface> m_audio)
{
    LOG_TRACE("[TimerGUI] Initializing");
    this->m_renderer = m_renderer;
    this->m_audio = m_audio;
    this->m_assets = m_assets;
}

TimerGUI::~TimerGUI()
{

    LOG_TRACE("[TimerGUI] Destroy");
    this->m_renderer->freeTexture("header_bar");
}

// ============================================================
// Public Methods
// ============================================================

void TimerGUI::onEnter()
{
    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/images/gui/header_bar.png", w, h, c);
    this->m_renderer->createTexture("header_bar", pixel, w, h);
}

void TimerGUI::onDraw()
{
    this->m_renderer->oxDrawSprite(0, 0, 1280, 58, "header_bar", Color::White(), 1);

    // ── Turn timer bar ────────────────────────────────────────────────────────
    if (m_turn_active) {
        float ratio = m_turn_left / m_turn_duration;
        float bar_w = 280.0f;
        float bar_h = 14.0f;
        float bar_x = CENTER_SCREEN - bar_w * 0.5f;
        float bar_y = 100.0f;

        // background track
        m_renderer->oxDrawRectangle(bar_x - 2, bar_y - 2, bar_w + 4, bar_h + 4,
                                    Color(20, 20, 20, 180), 4, 0, 0, 0);

        // fill: green → yellow → red
        Color fill = ratio > 0.5f    ? Color(50, 200, 60, 255)
                     : ratio > 0.25f ? Color(230, 170, 0, 255)
                                     : Color(220, 40, 40, 255);
        if (ratio > 0.0f)
            m_renderer->oxDrawRectangle(bar_x, bar_y, bar_w * ratio, bar_h, fill, 5, 0, 0, 0);
    }
}

void TimerGUI::onUpdate(double dt)
{
    this->m_timer += dt;
    if (this->m_timer >= 1) {
        if (this->m_countdown) {
            this->m_count -= 1;
            this->fontSizeCount = 84;
            if (this->m_count <= 0) {
                this->m_countdown = false;
                this->timeUp();
            }
        }
        this->m_time += 1;
        this->m_timer = 0;
    }

    if (this->m_countdown) {
        float targetSize = 52.0f;
        float speed = 10.0f;
        this->fontSizeCount += (targetSize - this->fontSizeCount) * speed * dt;
    }

    // ── Turn timer ────────────────────────────────────────────────────────────
    if (m_turn_active) {
        m_turn_left -= static_cast<float>(dt);
        if (m_turn_left <= 0.0f) {
            m_turn_left = 0.0f;
            m_turn_active = false;
            m_turn_expired = true;
        }
    }
}

// ============================================================
// Private Methods
// ============================================================

void TimerGUI::startTimer(int count)
{
    this->m_timer = 0;
    this->m_count = count;
    this->m_countdown = true;
}

void TimerGUI::stopTimer()
{
    this->m_countdown = false;
}

void TimerGUI::timeUp() {}

void TimerGUI::startTurn(float duration)
{
    m_turn_duration = duration;
    m_turn_left = duration;
    m_turn_active = true;
    m_turn_expired = false;
}

void TimerGUI::stopTurn()
{
    m_turn_active = false;
}

bool TimerGUI::isExpired()
{
    if (m_turn_expired) {
        m_turn_expired = false;
        return true;
    }
    return false;
}

std::string TimerGUI::formatTime(int totalSeconds)
{
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":"
        << std::setw(2) << seconds;

    return oss.str();
}
