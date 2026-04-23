#ifndef C26556A7_847B_4BF4_A398_40AC8C5226EA
#define C26556A7_847B_4BF4_A398_40AC8C5226EA

#include "renderer/ox_renderer.h"
#include <memory>

struct MainMenuParticle
{
    float x = 0, y = 0;
    float vx = 0, vy = 0;

    float lifetime = 0;
    float max_lifetime = 1.0f;

    float size = 6.0f;
    float growth = 0.0f; // size increase over time

    float sway_phase = 0.0f;
    float sway_speed = 0.0f;
    float sway_amp = 0.0f;

    uint8_t r = 120, g = 160, b = 255;

    int layer_offset = 0;
    bool active = false;
};

class MainMenuParticleEmitter
{
  public:
    static constexpr int POOL_SIZE = 128;

    void update(double dt, float screen_w, float bottom_y, bool emitting);

    void draw(std::shared_ptr<OxRenderer> renderer, int32_t base_layer) const;

    void clear();

  private:
    MainMenuParticle *acquire();
    void spawn(float screen_w, float bottom_y);

    static float frand(float lo, float hi);

    std::array<MainMenuParticle, POOL_SIZE> m_pool{};
    float m_emit_timer = 0.0f;

    static constexpr float EMIT_INTERVAL = 0.05f; // ความถี่ spawn
    static constexpr int MAX_PARTICLES = 30;
};

#endif /* C26556A7_847B_4BF4_A398_40AC8C5226EA */
