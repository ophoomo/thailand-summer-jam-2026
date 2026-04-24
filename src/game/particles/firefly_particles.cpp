
#include "game/particles/firefly_particles.h"

static constexpr float PI = 3.14159265f;

// ============================================================
// Helpers
// ============================================================

float FireflyParticleEmitter::frand(float lo, float hi)
{
    return lo + static_cast<float>(rand() % 10000) / 10000.0f * (hi - lo);
}

FireflyParticle *FireflyParticleEmitter::acquire()
{
    int activeCount = 0;

    for (auto &p : m_pool) {
        if (p.active)
            activeCount++;
    }

    if (activeCount >= MAX_PARTICLES)
        return nullptr;

    for (auto &p : m_pool)
        if (!p.active)
            return &p;

    return nullptr;
}

// ============================================================
// Spawn — from bottom, random X within width
// ============================================================

void FireflyParticleEmitter::spawn(float screen_w, float screen_h)
{
    FireflyParticle *p = acquire();
    if (!p)
        return;

    p->x = frand(0.0f, screen_w);
    p->y = frand(0.0f, screen_h);

    p->vx = frand(-1.0f, 1.0f);
    p->vy = frand(-1.0f, 1.0f);

    p->lifetime = frand(5.0f, 10.0f);
    p->max_lifetime = p->lifetime;

    p->size = frand(1.0f, 2.5f);
    p->growth = frand(0.0f, 0.15f);

    // movement noise
    p->sway_phase = frand(0.0f, PI * 2.0f);
    p->sway_speed = frand(0.5f, 2.0f);

    // Blue group
    int t = rand() % 3;

    if (t == 0) {
        p->r = 70;
        p->g = 130;
        p->b = 255;
    } else if (t == 1) {
        p->r = 10;
        p->g = 40;
        p->b = 120;
    } else {
        p->r = 255;
        p->g = 255;
        p->b = 255;
    }

    p->layer_offset = (rand() % 2) ? 10 : 0;

    p->active = true;
}

// ============================================================
// Update
// ============================================================

void FireflyParticleEmitter::update(double dt, float screen_w, float screen_h, bool emitting)
{
    float fdt = static_cast<float>(dt);

    for (auto &p : m_pool) {
        if (!p.active)
            continue;

        p.lifetime -= fdt;

        // recycle แทนการหาย
        if (p.lifetime <= 0.0f) {
            p.x = frand(0.0f, screen_w);
            p.y = frand(0.0f, screen_h);
            p.lifetime = p.max_lifetime;
        }

        // noise movement (หิ่งห้อยส่าย)
        p.sway_phase += p.sway_speed * fdt;

        float nx = std::sin(p.sway_phase * 1.7f);
        float ny = std::cos(p.sway_phase * 1.3f);

        p.vx += nx * 0.6f * fdt;
        p.vy += ny * 0.6f * fdt;

        // damping กันพุ่ง
        p.vx *= 0.97f;
        p.vy *= 0.97f;

        p.x += p.vx;
        p.y += p.vy;

        // wrap screen (ลอยออกแล้ววนกลับ)
        if (p.x < 0)
            p.x = screen_w;
        if (p.x > screen_w)
            p.x = 0;
        if (p.y < 0)
            p.y = screen_h;
        if (p.y > screen_h)
            p.y = 0;

        p.size += p.growth * fdt;
    }

    if (emitting) {
        m_emit_timer -= fdt;
        if (m_emit_timer <= 0.0f) {
            spawn(screen_w, screen_h);
            m_emit_timer = EMIT_INTERVAL;
        }
    }
}

// ============================================================
// Draw
// ============================================================

void FireflyParticleEmitter::draw(std::shared_ptr<OxRenderer> renderer, int32_t base_layer) const
{
    for (const auto &p : m_pool) {
        if (!p.active)
            continue;

        float t = p.lifetime / p.max_lifetime;

        // fade
        uint8_t alpha = static_cast<uint8_t>(std::clamp(std::pow(t, 0.6f) * 180.0f, 0.0f, 180.0f));

        if (alpha < 2)
            continue;

        // FIREFLY FLICKER
        float flicker = 0.6f + 0.4f * std::sin(p.sway_phase * 6.0f) + frand(-0.15f, 0.15f);

        flicker = std::clamp(flicker, 0.2f, 1.0f);

        alpha = static_cast<uint8_t>(alpha * flicker);

        int32_t layer = base_layer + p.layer_offset;

        float size = p.size;

        // glow outer
        renderer->oxDrawCircle(p.x, p.y, size * 2.2f,
                               {p.r, p.g, p.b, static_cast<uint8_t>(alpha / 6)}, layer);

        // core light
        renderer->oxDrawCircle(p.x, p.y, size, {p.r, p.g, p.b, alpha}, layer);
    }
}

// ============================================================
// Clear
// ============================================================

void FireflyParticleEmitter::clear()
{
    for (auto &p : m_pool)
        p.active = false;
}
