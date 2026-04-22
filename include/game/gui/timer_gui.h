#ifndef BEE808D3_55DF_4066_A0A9_C3F365D7D8C2
#define BEE808D3_55DF_4066_A0A9_C3F365D7D8C2

#include "assets/assets_interface.h"
#include "audio/audio_interface.h"
#include "renderer/ox_renderer.h"
#include <memory>

static constexpr float CENTER_SCREEN = 640.0f;

class TimerGUI
{
  public:
    TimerGUI(std::shared_ptr<OxRenderer> m_renderer, std::shared_ptr<AssetsInterface> m_assets,
             std::shared_ptr<AudioInterface> m_audio);
    ~TimerGUI();

    void onEnter();
    void onDraw();
    void onUpdate(double dt);

    // ── Turn timer ────────────────────────────────────────────────────────────
    void startTurn(float duration = 10.0f);
    void stopTurn();
    bool isExpired(); // returns true once then auto-resets

  private:
    void startTimer(int count);
    void stopTimer();
    void timeUp();
    std::string formatTime(int totalSeconds);

    std::shared_ptr<OxRenderer> m_renderer;
    std::shared_ptr<AudioInterface> m_audio;
    std::shared_ptr<AssetsInterface> m_assets;

    float m_timer = 0;
    bool m_countdown = false;
    int m_count = 0;
    int m_time = 0;
    float fontSizeCount = 52;

    // ── Turn timer state ──────────────────────────────────────────────────────
    float m_turn_left{0.0f};
    float m_turn_duration{10.0f};
    bool m_turn_active{false};
    bool m_turn_expired{false};
};

#endif /* BEE808D3_55DF_4066_A0A9_C3F365D7D8C2 */
