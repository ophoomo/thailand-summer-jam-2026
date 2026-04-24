
#include "game/particles/header_particles.h"

static constexpr float PI = 3.14159265f;

// ============================================================
// Helpers
// ============================================================

float HeaderParticleEmitter::frand(float lo, float hi)
{
    return lo + static_cast<float>(rand() % 10000) / 10000.0f * (hi - lo);
}

HeaderParticle *HeaderParticleEmitter::acquire()
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

void HeaderParticleEmitter::spawn(float screen_w, float bottom_y)
{
    HeaderParticle *p = acquire();
    if (!p)
        return;

    p->x = frand(0.0f, screen_w);
    p->y = frand(-20.0f, 0.0f);

    p->vx = frand(-1.0f, 1.0f);
    p->vy = frand(10.0f, 25.0f); // ลงล่าง

    p->lifetime = frand(3.5f, 6.5f);
    p->max_lifetime = p->lifetime;

    p->size = frand(1.0f, 1.8f);
    p->growth = frand(0.2f, 0.8f);

    p->sway_phase = frand(0.0f, PI * 2.0f);
    p->sway_speed = frand(0.3f, 1.0f);
    p->sway_amp = frand(1.0f, 4.0f);

    p->r = 255;
    p->g = 255;
    p->b = 255;

    int r = rand() % 10;
    if (r < 4) {
        p->layer_offset = 10;
    }

    p->flicker_phase = frand(0.0f, PI * 2.0f);
    p->flicker_speed = frand(6.0f, 12.0f);

    p->active = true;
}

// ============================================================
// Update
// ============================================================

void HeaderParticleEmitter::update(double dt, float screen_w, float bottom_y, bool emitting)
{
    float fdt = static_cast<float>(dt);

    for (auto &p : m_pool) {
        if (!p.active)
            continue;

        p.lifetime -= fdt;
        if (p.lifetime <= 0.0f) {
            p.active = false;
            continue;
        }

        // sway motion
        p.sway_phase += p.sway_speed * fdt;
        float sway = std::sin(p.sway_phase) * p.sway_amp * 0.3f;

        float noise = std::sin(p.sway_phase * 1.3f + p.x * 0.01f) * 0.5f;
        float noise2 = std::sin(p.sway_phase * 0.7f + p.y * 0.02f) * 0.5f;

        p.x += (p.vx + sway + noise) * fdt;
        p.y += (p.vy + noise2) * fdt;

        p.x += (p.vx + sway) * fdt;
        p.y += p.vy * fdt;

        // grow over time
        p.size += p.growth * fdt;

        p.flicker_phase += p.flicker_speed * fdt;
    }

    if (emitting) {
        m_emit_timer -= fdt;
        if (m_emit_timer <= 0.0f) {
            spawn(screen_w, bottom_y);
            m_emit_timer = EMIT_INTERVAL;
        }
    }
}

// ============================================================
// Draw
// ============================================================

void HeaderParticleEmitter::draw(std::shared_ptr<OxRenderer> renderer, int32_t base_layer) const
{
    for (const auto &p : m_pool) {
        if (!p.active)
            continue;

        float flicker = 0.6f + 0.4f * std::sin(p.flicker_phase);
        float t = (p.lifetime / p.max_lifetime) * flicker;

        uint8_t alpha = static_cast<uint8_t>(std::clamp(t * 180.0f, 0.0f, 180.0f));

        if (alpha < 2)
            continue;

        float size = p.size;

        int32_t layer = base_layer + p.layer_offset;

        renderer->oxDrawCircle(p.x, p.y, size * 2.5f,
                               {255, 255, 255, static_cast<uint8_t>(alpha / 6)}, layer);

        renderer->oxDrawCircle(p.x, p.y, size, {p.r, p.g, p.b, alpha}, layer);
    }
}

// ============================================================
// Clear
// ============================================================

void HeaderParticleEmitter::clear()
{
    for (auto &p : m_pool)
        p.active = false;
}
