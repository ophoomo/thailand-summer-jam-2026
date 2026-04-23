
#include "game/particles/background_particles.h"

static constexpr float PI = 3.14159265f;

// ============================================================
// Helpers
// ============================================================

float BackgroundParticleEmitter::frand(float lo, float hi)
{
    return lo + static_cast<float>(rand() % 10000) / 10000.0f * (hi - lo);
}

BGParticle *BackgroundParticleEmitter::acquire()
{
    for (auto &p : m_pool)
        if (!p.active)
            return &p;
    return nullptr;
}

// ============================================================
// Spawn — from bottom, random X within width
// ============================================================

void BackgroundParticleEmitter::spawn(float screen_w, float bottom_y)
{
    BGParticle *p = acquire();
    if (!p)
        return;

    p->x = frand(0.0f, screen_w);
    p->y = bottom_y + frand(0.0f, 10.0f);

    p->vx = frand(-3.0f, 3.0f);
    p->vy = -frand(10.0f, 25.0f);

    p->lifetime = frand(3.5f, 6.5f);
    p->max_lifetime = p->lifetime;

    p->size = frand(1.2f, 2.8f);
    p->growth = frand(0.2f, 0.8f);

    p->sway_phase = frand(0.0f, PI * 2.0f);
    p->sway_speed = frand(0.3f, 1.0f);
    p->sway_amp = frand(1.0f, 4.0f);

    int tier = rand() % 10;
    if (tier < 4) {
        // deep purple
        p->r = 70;
        p->g = 40;
        p->b = 120;
    } else if (tier < 8) {
        // mid purple
        p->r = 140;
        p->g = 90;
        p->b = 200;
    } else {
        // soft lavender glow
        p->r = 200;
        p->g = 170;
        p->b = 255;
    }

    int r = rand() % 10;
    if (r < 4) {
        p->layer_offset = 10;
    }

    p->active = true;
}

// ============================================================
// Update
// ============================================================

void BackgroundParticleEmitter::update(double dt, float screen_w, float bottom_y, bool emitting)
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
        float sway = std::sin(p.sway_phase) * p.sway_amp;

        p.x += (p.vx + sway) * fdt;
        p.y += p.vy * fdt;

        // grow over time
        p.size += p.growth * fdt;
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

void BackgroundParticleEmitter::draw(std::shared_ptr<OxRenderer> renderer, int32_t base_layer) const
{
    for (const auto &p : m_pool) {
        if (!p.active)
            continue;

        float t = p.lifetime / p.max_lifetime;

        // นุ่มขึ้น + fade ช้า
        uint8_t alpha = static_cast<uint8_t>(std::clamp(std::pow(t, 0.6f) * 140.0f, 0.0f, 140.0f));

        if (alpha < 2)
            continue;

        float size = p.size;

        int32_t layer = base_layer + p.layer_offset;

        renderer->oxDrawCircle(p.x, p.y, size * 1.6f,
                               {p.r, p.g, p.b, static_cast<uint8_t>(alpha / 5)}, layer);

        renderer->oxDrawCircle(p.x, p.y, size, {p.r, p.g, p.b, alpha}, layer);
    }
}

// ============================================================
// Clear
// ============================================================

void BackgroundParticleEmitter::clear()
{
    for (auto &p : m_pool)
        p.active = false;
}
