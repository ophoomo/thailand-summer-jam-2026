#ifndef C270AA4F_4FA2_404B_B1E1_AB5D40D8C0E3
#define C270AA4F_4FA2_404B_B1E1_AB5D40D8C0E3

#include "game/cards/card_info.h"
#include "game/cards/card_particles.h"
#include "game/utils/math_helper.h"
#include "renderer/ox_renderer.h"
#include <cstdint>
#include <string>

enum class CardState { IDLE, DRAWING, DISCARDING };

struct BobbingEffect
{
    float speed = 1.5f;
    float amplitude = 1.0f;
    double time_acc = 0.0;

    void randomize()
    {
        time_acc = static_cast<double>(rand() % 1000) / 10.0;

        float variation = ((rand() % 40) - 20) / 100.0f;
        speed += variation;
    }

    float update(double dt)
    {
        time_acc += dt;
        return std::sin(time_acc * speed) * amplitude;
    }
};

class Card
{
  public:
    Card(std::shared_ptr<OxRenderer> renderer);
    ~Card();

    void onEnter(CardInfo info, const float w, const float h);
    void onUpdate(double dt);
    void onDraw();

    void drawFrom(float x, float y);
    void startFlyIn(float from_y);
    void discard();
    bool checkHover(float mouseX, float mouseY, float pad = 0.0f);

    void setPosition(float x, float y)
    {
        this->base_x = x;
        this->base_y = y;
    }
    void setOnHover(bool status)
    {
        this->is_hovered = status;
    }
    bool getOnHover()
    {
        return this->is_hovered;
    }
    bool isVisible() const
    {
        return this->m_info.show;
    }
    bool isInHand() const
    {
        return this->m_info.show && this->m_state != CardState::DISCARDING;
    }
    void setSelected(bool s)
    {
        this->is_selected = s;
    }
    bool getSelected() const
    {
        return this->is_selected;
    }

    // Show this card (fly-in animation) with the given info
    void showCard(const CardInfo& info, float from_y = 1100.0f);
    // Trigger the discard animation (hides card)
    void hideCard();
    bool isDiscarding() const { return m_state == CardState::DISCARDING; }

    float getCenterX() const { return x + width * 0.5f; }
    float getCenterY() const { return y + height * 0.5f; }

  private:
    void updateAnimations(double dt);
    float lerp(float start, float end, float t)
    {
        return start + t * (end - start);
    }

    std::shared_ptr<OxRenderer> m_renderer;
    CardInfo m_info;
    CardState m_state = CardState::IDLE;

    // Positions & Transforms
    float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f;
    float base_x = 0.0f, base_y = 0.0f;
    float start_x = 0.0f, start_y = 0.0f;

    // Animation states
    float m_anim_time = 0.0f;
    float m_scale = 1.0f;
    float m_target_scale = 1.0f;
    uint8_t m_opacity = 255;
    bool is_hovered = false;

    bool is_selected = false;
    bool m_prev_selected = false;
    float m_y_offset = 0.0f;
    float m_glow_intensity = 0.0f;
    float m_glow_alpha = 0.0f;

    // Explosion / discard
    float m_rotation = 0.0f;
    float m_discard_dir_x = 1.0f;
    bool m_burst_fired = false;

    Sway sway_x;
    Hover hover_y;
    CardOscillator glow_osc;
    CardParticleEmitter m_particles;
};

#endif /* C270AA4F_4FA2_404B_B1E1_AB5D40D8C0E3 */
