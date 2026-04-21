
#include "game/cards/card_particles.h"
#include "renderer/color.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

static constexpr float PI = 3.14159265f;

// ============================================================
// Helpers
// ============================================================

float CardParticleEmitter::frand(float lo, float hi)
{
    return lo + static_cast<float>(rand() % 10000) / 10000.0f * (hi - lo);
}

SmokeParticle *CardParticleEmitter::acquireParticle()
{
    for (auto &p : m_pool) {
        if (!p.active)
            return &p;
    }
    return nullptr;
}

// Three-tier dark-blue palette
void CardParticleEmitter::applyPalette(SmokeParticle &p)
{
    int tier = rand() % 10;
    if (tier < 4) {
        p.r = 15;
        p.g = 20;
        p.b = 85; // deep navy
    } else if (tier < 8) {
        p.r = 30;
        p.g = 55;
        p.b = 165; // mid blue
    } else {
        p.r = 90;
        p.g = 140;
        p.b = 235; // bright sparkle
    }
}

// ============================================================
// Burst — fires once on selection
// ============================================================

void CardParticleEmitter::burst(float cx, float cy, float card_w, float card_h)
{
    constexpr int COUNT = 26;

    for (int i = 0; i < COUNT; i++) {
        SmokeParticle *p = acquireParticle();
        if (!p)
            return;

        float angle = frand(0.0f, PI * 2.0f);
        float spawn_r = frand(0.0f, 1.0f);
        p->x = cx + std::cos(angle) * spawn_r * card_w * 0.45f;
        p->y = cy + std::sin(angle) * spawn_r * card_h * 0.45f;

        float speed = frand(70.0f, 180.0f);
        p->vx = std::cos(angle) * speed * frand(0.5f, 1.0f);
        p->vy = std::sin(angle) * speed * frand(0.5f, 1.0f) - frand(30.0f, 80.0f);

        p->lifetime = frand(0.5f, 1.4f);
        p->max_lifetime = p->lifetime;
        p->size = frand(4.0f, 18.0f);
        p->drag = frand(1.2f, 2.8f);
        p->front = (rand() % 10) >= 4; // 60% in front, 40% behind
        applyPalette(*p);
        p->active = true;
    }
}

// ============================================================
// Orbital ambient spawn
//
// Particles spawn on the card's elliptical perimeter and are
// given a proper ellipse-tangent velocity so they orbit the
// card shape.  Low drag lets them glide for 1-2 seconds before
// fading.  The 'front' flag splits them between layer behind
// and layer in-front-of the card at draw time.
// ============================================================

void CardParticleEmitter::spawnOrbital(float cx, float cy, float card_w, float card_h)
{
    SmokeParticle *p = acquireParticle();
    if (!p)
        return;

    // Semi-axes: slightly outside card bounds with random margin
    float rx = card_w * 0.52f + frand(6.0f, 24.0f);
    float ry = card_h * 0.52f + frand(6.0f, 24.0f);

    float angle = frand(0.0f, PI * 2.0f);
    p->x = cx + std::cos(angle) * rx;
    p->y = cy + std::sin(angle) * ry;

    // Ellipse tangent direction: (-rx*sinθ, ry*cosθ) / |...|
    float tx = -rx * std::sin(angle);
    float ty = ry * std::cos(angle);
    float tlen = std::sqrt(tx * tx + ty * ty);

    float dir = (rand() % 2) ? 1.0f : -1.0f; // CCW or CW
    float orbit_speed = frand(45.0f, 95.0f);
    p->vx = dir * orbit_speed * tx / tlen;
    p->vy = dir * orbit_speed * ty / tlen;

    p->lifetime = frand(1.0f, 2.4f);
    p->max_lifetime = p->lifetime;
    p->size = frand(3.0f, 12.0f);
    p->drag = frand(0.25f, 0.75f); // low drag — maintain orbit longer
    p->front = (rand() % 10) >= 3; // 70% in front, 30% behind card
    applyPalette(*p);
    p->active = true;
}

// ============================================================
// Clear — kill all active particles instantly
// ============================================================

void CardParticleEmitter::clear()
{
    for (auto &p : m_pool)
        p.active = false;
}

// ============================================================
// Explosion burst — warm orange / gold / white palette.
// Fires when a card is discarded or played.
// ============================================================

void CardParticleEmitter::explodeBurst(float cx, float cy, float card_w, float card_h)
{
    constexpr int COUNT = 32;

    for (int i = 0; i < COUNT; i++) {
        SmokeParticle *p = acquireParticle();
        if (!p)
            return;

        float angle = frand(0.0f, PI * 2.0f);
        float spawn_r = frand(0.0f, 1.0f);
        p->x = cx + std::cos(angle) * spawn_r * card_w * 0.48f;
        p->y = cy + std::sin(angle) * spawn_r * card_h * 0.48f;

        // High-speed radial burst
        float speed = frand(120.0f, 320.0f);
        p->vx = std::cos(angle) * speed;
        p->vy = std::sin(angle) * speed - frand(20.0f, 70.0f); // slight upward bias

        p->lifetime = frand(0.25f, 0.75f);
        p->max_lifetime = p->lifetime;
        p->size = frand(3.0f, 15.0f);
        p->drag = frand(2.5f, 5.0f); // high drag — sharp deceleration
        p->front = true;              // always on top for dramatic effect

        // Moonlight palette: deep navy / mid blue / icy white spark
        int tier = rand() % 10;
        if (tier < 4) {
            p->r = 20;
            p->g = 40;
            p->b = 160; // deep moonlight navy
        } else if (tier < 8) {
            p->r = 80;
            p->g = 160;
            p->b = 255; // bright ice blue
        } else {
            p->r = 210;
            p->g = 235;
            p->b = 255; // near-white moonbeam
        }

        p->active = true;
    }
}

// ============================================================
// Update
// ============================================================

void CardParticleEmitter::update(double dt, float cx, float cy, float card_w, float card_h,
                                 bool emitting)
{
    const float fdt = static_cast<float>(dt);

    for (auto &p : m_pool) {
        if (!p.active)
            continue;

        p.lifetime -= fdt;
        if (p.lifetime <= 0.0f) {
            p.active = false;
            continue;
        }

        float drag_factor = 1.0f - p.drag * fdt;
        if (drag_factor < 0.0f)
            drag_factor = 0.0f;
        p.vx *= drag_factor;
        p.vy *= drag_factor;

        p.x += p.vx * fdt;
        p.y += p.vy * fdt;
    }

    if (emitting) {
        m_emit_timer -= fdt;
        if (m_emit_timer <= 0.0f) {
            spawnOrbital(cx, cy, card_w, card_h);
            m_emit_timer = EMIT_INTERVAL;
        }
    }
}

// ============================================================
// Draw — three-ring soft glow, split across two layers
//   back_layer  : particles rendered behind the card
//   front_layer : particles rendered in front of the card
// ============================================================

void CardParticleEmitter::draw(std::shared_ptr<OxRenderer> renderer, int32_t back_layer,
                               int32_t front_layer) const
{
    for (const auto &p : m_pool) {
        if (!p.active)
            continue;

        float t = p.lifetime / p.max_lifetime;
        float alpha_f = std::sqrt(t) * 210.0f;
        auto alpha = static_cast<uint8_t>(std::clamp(alpha_f, 0.0f, 210.0f));
        if (alpha < 3)
            continue;

        int32_t layer = p.front ? front_layer : back_layer;
        float cur_size = p.size * (0.3f + 0.7f * t);

        // Outer halo
        renderer->oxDrawCircle(p.x, p.y, cur_size * 2.0f,
                               {p.r, p.g, p.b, static_cast<uint8_t>(alpha / 5)}, layer);
        // Mid glow
        renderer->oxDrawCircle(p.x, p.y, cur_size * 1.4f,
                               {p.r, p.g, p.b, static_cast<uint8_t>(alpha / 2)}, layer);
        // Bright core
        renderer->oxDrawCircle(p.x, p.y, cur_size, {p.r, p.g, p.b, alpha}, layer);
    }
}
