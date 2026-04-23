#ifndef E68DB9F2_7227_40EE_AC74_2E7F046C1738
#define E68DB9F2_7227_40EE_AC74_2E7F046C1738

#include "renderer/ox_renderer.h"
#include <array>
#include <cstdint>
#include <memory>

// ============================================================
// BottomParticle — floating background circle
// ============================================================
struct BottomParticle
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

// ============================================================
// BottomParticleEmitter
//   - spawn from bottom, float upward, fade out
// ============================================================
class BottomParticleEmitter
{
  public:
    static constexpr int POOL_SIZE = 128;

    void update(double dt, float screen_w, float bottom_y, bool emitting);

    void draw(std::shared_ptr<OxRenderer> renderer, int32_t base_layer) const;

    void clear();

  private:
    BottomParticle *acquire();
    void spawn(float screen_w, float bottom_y);

    static float frand(float lo, float hi);

    std::array<BottomParticle, POOL_SIZE> m_pool{};
    float m_emit_timer = 0.0f;

    static constexpr float EMIT_INTERVAL = 0.08f; // ความถี่ spawn
    static constexpr int MAX_PARTICLES = 50;
};

#endif /* E68DB9F2_7227_40EE_AC74_2E7F046C1738 */
