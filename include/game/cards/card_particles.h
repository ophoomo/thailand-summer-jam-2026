#ifndef A3F812CC_7D4B_4E11_9A2B_0C5D6F3E8B12
#define A3F812CC_7D4B_4E11_9A2B_0C5D6F3E8B12

#include "renderer/ox_renderer.h"
#include <array>
#include <memory>

// ============================================================
// SmokeParticle — a single dark-blue smoke/glow puff
// ============================================================
struct SmokeParticle
{
    float x = 0, y = 0;
    float vx = 0, vy = 0;
    float lifetime = 0;
    float max_lifetime = 1.0f;
    float size = 5.0f;
    float drag = 1.5f;
    uint8_t r = 30, g = 50, b = 140;
    bool active = false;
    bool front = true; // true = rendered above card, false = rendered behind card
};

// ============================================================
// CardParticleEmitter
//   - burst()  : explosion of particles on card selection
//   - update() : advances simulation; handles ambient emission
//   - draw()   : renders all active particles
// ============================================================
class CardParticleEmitter
{
  public:
    static constexpr int POOL_SIZE = 96;

    void burst(float cx, float cy, float card_w, float card_h);
    void explodeBurst(float cx, float cy, float card_w, float card_h);
    void clear();
    void update(double dt, float cx, float cy, float card_w, float card_h, bool emitting);
    void draw(std::shared_ptr<OxRenderer> renderer, int32_t back_layer,
              int32_t front_layer) const;

  private:
    SmokeParticle *acquireParticle();
    void spawnOrbital(float cx, float cy, float card_w, float card_h);
    static float frand(float lo, float hi);
    static void applyPalette(SmokeParticle &p);

    std::array<SmokeParticle, POOL_SIZE> m_pool{};
    float m_emit_timer = 0.0f;
    static constexpr float EMIT_INTERVAL = 0.10f; // ~10 ambient particles / sec
};

#endif /* A3F812CC_7D4B_4E11_9A2B_0C5D6F3E8B12 */
