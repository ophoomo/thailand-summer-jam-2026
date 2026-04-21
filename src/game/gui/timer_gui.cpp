
#include "game/gui/timer_gui.h"
#include "renderer/color.h"
#include "renderer/text_effect.h"
#include "utils/logger.h"
#include <string>

// ============================================================
// Construction / destruction
// ============================================================

TimerGUI::TimerGUI(std::shared_ptr<OxRenderer> m_renderer, std::shared_ptr<AudioInterface> m_audio)
{
    LOG_TRACE("[TimerGUI] Initializing");
    this->m_renderer = m_renderer;
    this->m_audio = m_audio;
}

TimerGUI::~TimerGUI()
{

    LOG_TRACE("[TimerGUI] Destroy");
}

// ============================================================
// Public Methods
// ============================================================

void TimerGUI::onEnter() {}

void TimerGUI::onDraw()
{
    if (this->m_countdown) {
        auto textCountdown = std::to_string(this->m_count);
        float pos_x =
            CENTER_SCREEN -
            (this->m_renderer->measureText(textCountdown.c_str(), this->fontSizeCount) / 2);
        this->m_renderer->oxDrawText(pos_x, 120, textCountdown.c_str(), this->fontSizeCount,
                                     Color::Red(), TextEffect::None());
    }

    int font_size = 20;
    auto textTimer = this->formatTime(this->m_time);
    int pos_x = CENTER_SCREEN - (this->m_renderer->measureText(textTimer.c_str(), font_size) / 2);
    this->m_renderer->oxDrawText(pos_x, 40, textTimer.c_str(), font_size, Color::White(),
                                 TextEffect::None());

    // ── Turn timer bar ────────────────────────────────────────────────────────
    if (m_turn_active) {
        float ratio  = m_turn_left / m_turn_duration;
        float bar_w  = 280.0f;
        float bar_h  = 14.0f;
        float bar_x  = CENTER_SCREEN - bar_w * 0.5f;
        float bar_y  = 72.0f;

        // background track
        m_renderer->oxDrawRectangle(bar_x - 2, bar_y - 2, bar_w + 4, bar_h + 4,
                                    Color(20, 20, 20, 180), 4, 0, 0, 0);

        // fill: green → yellow → red
        Color fill = ratio > 0.5f  ? Color(50,  200, 60,  255)
                   : ratio > 0.25f ? Color(230, 170, 0,   255)
                                   : Color(220, 40,  40,  255);
        if (ratio > 0.0f)
            m_renderer->oxDrawRectangle(bar_x, bar_y, bar_w * ratio, bar_h, fill, 5, 0, 0, 0);

        // remaining seconds label
        auto label = std::to_string(static_cast<int>(std::ceil(m_turn_left)));
        float tx = CENTER_SCREEN - m_renderer->measureText(label.c_str(), 16) * 0.5f;
        m_renderer->oxDrawText(tx, bar_y + bar_h + 3.0f, label.c_str(), 16,
                               Color::White(), TextEffect::None());
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
            m_turn_left    = 0.0f;
            m_turn_active  = false;
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
    m_turn_left     = duration;
    m_turn_active   = true;
    m_turn_expired  = false;
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
