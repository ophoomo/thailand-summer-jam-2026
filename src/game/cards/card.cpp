
#include "game/cards/card.h"
#include "game/cards/hand.h"
#include "renderer/text_effect.h"
#include "utils/logger.h"
#include <sstream>
#include <vector>
#include "core/localization.h"

// ============================================================
// Construction / destruction
// ============================================================

Card::Card(std::shared_ptr<OxRenderer> m_renderer)
{
    LOG_TRACE("[Card] Initializing");
    this->m_renderer = m_renderer;
}

Card::~Card()
{
    LOG_TRACE("[Card] Destroy");
}

// ============================================================
// Public Methods
// ============================================================

void Card::onEnter(CardInfo info, const float w, const float h)
{
    this->m_info = info;
    this->width = w;
    this->height = h;

    this->sway_x.randomize();
    this->hover_y.randomize();
    this->glow_osc.amplitude = 30.0f;
    this->glow_osc.frequency = 1.5f;
    this->glow_osc.randomize();
}

void Card::onUpdate(double dt)
{
    this->updateAnimations(dt);
}

void Card::onDraw()
{
    int32_t base_layer = is_selected ? 8 : 7;

    const std::string display_name = !m_info.name_key.empty()
        ? Localization::get(m_info.name_key)
        : m_info.name;

    const std::string display_detail = !m_info.desc_key.empty()
        ? Localization::get(m_info.desc_key)
        : m_info.detail;


    // Lingering explosion particles are drawn even after the sprite disappears
    m_particles.draw(m_renderer, 1, base_layer + 2);

    if (!m_info.show)
        return;

    float draw_w = width * m_scale;
    float draw_h = height * m_scale;
    float draw_x = x - (draw_w - width) * 0.5f;
    float draw_y = y - (draw_h - height) * 0.5f;

    // ── Card sprite (golden tint when selected, spins on discard)
    float glow_t = std::clamp(m_glow_intensity / 150.0f, 0.0f, 1.0f);
    Color card_tint =
        Color::Lerp(Color{255, 255, 255, m_opacity}, Color{97, 156, 250, m_opacity}, glow_t);
    const std::string &art = m_info.art.empty() ? "card_skill" : m_info.art;
    m_renderer->oxDrawSprite(draw_x, draw_y, draw_w, draw_h, art.c_str(), card_tint, base_layer,
                             m_rotation);

    float ix = draw_x + 15.0f * m_scale;
    float iy = draw_y + 34.0f * m_scale;

    m_renderer->oxDrawSprite(ix, iy, INNER_WIDTH_CARD * m_scale, INNER_HEIGHT_CARD * m_scale,
                             m_info.inner, Color::White(), base_layer - 1);

    if (m_scale > 0.35f) {
        float text_cx = draw_x + draw_w * 0.5f;
        float text_lx = draw_x + draw_w * 0.5f;
        float text_max_w = draw_w * 0.86f;

        // ── Cost orb (deep moonlight blue, top-left corner)
        if (m_info.cost > 0) {
            float ocx = draw_x + draw_w * 0.5f;
            float ocy = draw_y + draw_h * 0.18f;
            std::string cs = std::to_string(static_cast<int>(m_info.cost));
            float ctw = m_renderer->measureText(cs.c_str(), 20);
            float lh = m_renderer->fontMetrics().lineHeight * 14.0f;
            m_renderer->oxDrawText(ocx - ctw * 0.5f, ocy - lh * 0.5f, cs.c_str(), 20,
                                   Color{200, 230, 255, 255},
                                   TextEffect::Outline(Color{10, 20, 60, 200}), base_layer + 1);
        }

        // ── Card name (moonlight-toned by type)
        Color name_col = Color{220, 240, 255, 255}; // default: cold white
        if (!is_selected) {
            if (m_info.type == "attack")
                name_col = Color{180, 100, 230, 255}; // violet-crimson
            else if (m_info.type == "skill")
                name_col = Color{80, 200, 255, 255}; // ice blue
            else if (m_info.type == "power")
                name_col = Color{140, 210, 255, 255}; // pale moonlight
        }
        TextEffect name_fx = is_selected ? TextEffect::Glow(Color{120, 190, 255, 255}, 0.35f, 1.8f)
                                         : TextEffect::Outline(Color{10, 20, 60, 220});
        {
            float ntw = m_renderer->measureText(display_name.c_str(), 12);
            m_renderer->oxDrawText(text_cx - ntw * 0.5f, draw_y + 135.0f * m_scale,
                                   display_name.c_str(), 12, name_col, name_fx, base_layer + 1);
        }

        // ── Description with word-wrap (left-aligned, size 11)
        if (!display_detail.empty()) {
            const float desc_sz = 11.0f;
            const float line_h  = m_renderer->fontMetrics().lineHeight * desc_sz;

            std::vector<std::string> lines;
            std::string cur;
            std::istringstream ss(display_detail);
            std::string word;
            while (ss >> word) {
                std::string test = cur.empty() ? word : cur + " " + word;
                if (m_renderer->measureText(test.c_str(), desc_sz) <= text_max_w)
                    cur = test;
                else {
                    if (!cur.empty()) lines.push_back(cur);
                    cur = word;
                }
            }
            if (!cur.empty()) lines.push_back(cur);

            for (int li = 0; li < static_cast<int>(lines.size()); li++) {
                float ly = draw_y + (160.0f + li * line_h) * m_scale;
                float tw = m_renderer->measureText(lines[li].c_str(), desc_sz);
                m_renderer->oxDrawText(text_cx - tw * 0.5f, ly, lines[li].c_str(), desc_sz,
                                       Color::Black(), TextEffect::Outline(Color::Black()),
                                       base_layer + 1);
            }
        }
    }
}

void Card::showCard(const CardInfo &info, float from_y)
{
    if (m_info.show && m_info.id == info.id)
        return;
    m_particles.clear(); // kill any lingering discard/explosion particles
    m_burst_fired = false;
    m_info = info;
    m_info.show = true;
    m_state = CardState::IDLE;
    m_opacity = 255;
    m_scale = 1.0f;
    m_rotation = 0.0f;
    startFlyIn(from_y);
}

void Card::hideCard()
{
    if (!m_info.show || m_state == CardState::DISCARDING)
        return;
    discard();
}

void Card::drawFrom(float x, float y)
{
    this->start_x = x;
    this->start_y = y;
    this->x = x;
    this->y = y;
    this->m_anim_time = 0.0f;
    this->m_state = CardState::DRAWING;
}

void Card::startFlyIn(float from_y)
{
    drawFrom(base_x, from_y);
}

void Card::showCardInPlace(const CardInfo &info)
{
    m_particles.clear();
    m_burst_fired = false;
    m_info = info;
    m_info.show = true;
    m_state = CardState::IDLE;
    m_opacity = 255;
    m_scale = 1.0f;
    m_rotation = 0.0f;
    is_hovered = false;
    is_selected = false;
    m_prev_selected = false;
    m_y_offset = 0.0f;
    m_glow_intensity = 0.0f;
    m_glow_alpha = 0.0f;
}

void Card::silentHide()
{
    m_info.show = false;
    m_state = CardState::IDLE;
    m_rotation = 0.0f;
    m_opacity = 255;
    m_scale = 1.0f;
    m_particles.clear();
    is_selected = false;
    m_prev_selected = false;
}

void Card::discard()
{
    if (this->m_state == CardState::DISCARDING)
        return;
    this->m_anim_time = 0.0f;
    this->m_state = CardState::DISCARDING;
    this->is_selected = false;
    this->m_prev_selected = false;
    this->m_burst_fired = false;
    this->m_discard_dir_x = (rand() % 2) ? 1.0f : -1.0f;
}

bool Card::checkHover(float mouseX, float mouseY, float pad)
{
    if (!m_info.show)
        return false;

    float draw_w = this->width * this->m_scale;
    float draw_h = this->height * this->m_scale;

    float draw_x = this->x - (draw_w - this->width) * 0.5f - pad;
    float draw_y = this->y - (draw_h - this->height) * 0.5f - pad;

    bool insideX = mouseX >= draw_x && mouseX <= (draw_x + draw_w + pad * 2.0f);
    bool insideY = mouseY >= draw_y && mouseY <= (draw_y + draw_h + pad * 2.0f);

    return insideX && insideY;
}

// ============================================================
// Private Methods
// ============================================================

void Card::updateAnimations(double dt)
{
    const float LERP_SPEED = 12.0f;
    const float fdt = static_cast<float>(dt);

    switch (m_state) {

    // ── DRAWING ─────────────────────────────────────────────
    case CardState::DRAWING: {
        m_anim_time += fdt * 1.5f;
        if (m_anim_time > 1.0f) {
            m_anim_time = 1.0f;
            m_state = CardState::IDLE;
        }
        float t = 1.0f - std::pow(1.0f - m_anim_time, 3);
        this->x = lerp(start_x, base_x, t);
        this->y = lerp(start_y, base_y, t);
        break;
    }

    // ── IDLE ────────────────────────────────────────────────
    case CardState::IDLE: {
        const float smooth = 1.0f - std::exp(-LERP_SPEED * fdt);

        m_target_scale = 1.0f + (is_hovered ? 0.15f : 0.0f) + (is_selected ? 0.05f : 0.0f);
        m_scale = lerp(m_scale, m_target_scale, smooth);

        float target_y_offset = (is_hovered ? -40.0f : 0.0f) + (is_selected ? -20.0f : 0.0f);
        m_y_offset = lerp(m_y_offset, target_y_offset, smooth);

        float target_glow = is_selected ? 150.0f : 0.0f;
        m_glow_intensity = lerp(m_glow_intensity, target_glow, smooth * 0.6f);
        float pulse = (m_glow_intensity > 1.0f) ? glow_osc.update(dt) : 0.0f;
        m_glow_alpha = std::clamp(m_glow_intensity + pulse, 0.0f, 255.0f);

        const float smooth_slide = 1.0f - std::exp(-6.0f * fdt);
        float target_x = base_x + sway_x.update(dt);
        float target_y = base_y + hover_y.update(dt) + m_y_offset;
        this->x = lerp(this->x, target_x, smooth_slide);
        this->y = lerp(this->y, target_y, smooth);

        {
            float dw = width * m_scale;
            float dh = height * m_scale;
            float pcx = x - (dw - width) * 0.5f + dw * 0.5f;
            float pcy = y - (dh - height) * 0.5f + dh * 0.5f;

            if (is_selected && !m_prev_selected)
                m_particles.burst(pcx, pcy, dw, dh);

            m_particles.update(dt, pcx, pcy, dw, dh, is_selected);
        }
        m_prev_selected = is_selected;
        break;
    }

    // ── DISCARDING — explosion effect ───────────────────────
    case CardState::DISCARDING: {
        m_anim_time += fdt * 1.6f; // ~0.625 s total

        // Kill orbital particles, fire explosion burst — exactly once
        if (!m_burst_fired) {
            m_burst_fired = true;
            m_particles.clear();
            float dw = width * m_scale;
            float dh = height * m_scale;
            float pcx = x - (dw - width) * 0.5f + dw * 0.5f;
            float pcy = y - (dh - height) * 0.5f + dh * 0.5f;
            m_particles.explodeBurst(pcx, pcy, dw, dh);
        }

        // Phase 1 (0 → 0.2): brief flash / scale-up
        if (m_anim_time < 0.2f) {
            float t = m_anim_time / 0.2f;
            m_scale = lerp(m_scale, 1.3f, t * 0.6f);
        } else {
            m_scale = lerp(m_scale, 0.0f, fdt * 9.0f);
            m_rotation += (4.0f + m_anim_time * 7.0f) * fdt;
        }

        // Fade opacity mid-animation
        float op_t = std::max(0.0f, 1.0f - m_anim_time * 1.3f);
        m_opacity = static_cast<uint8_t>(op_t * 255.0f);

        m_glow_intensity = lerp(m_glow_intensity, 0.0f, fdt * 8.0f);
        m_glow_alpha = std::clamp(m_glow_intensity, 0.0f, 255.0f);

        // Tick explosion particles so debris lingers after sprite disappears
        m_particles.update(dt, 0, 0, width, height, false);

        if (m_anim_time >= 1.0f) {
            m_info.show = false;
            m_rotation = 0.0f;
            m_opacity = 255;
            m_burst_fired = false;
            m_state = CardState::IDLE;
        }
        break;
    }
    }
}
