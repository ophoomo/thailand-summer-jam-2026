
#include "game/scenes/scene_game.h"
#include "core/scene_manager.h"
#include "engine/utils/logger.h"
#include "game/cards/hand.h"
#include "renderer/color.h"
#include "renderer/text_effect.h"
#include <SDL3/SDL_scancode.h>
#include <cmath>
#include <format>
#include <memory>
#include <vector>

// ============================================================
// Lifecycle
// ============================================================

void SceneGame::onEnter()
{
    LOG_TRACE("[SceneGame] Enter");
    this->m_dispatcher->sink<WindowMouseEvent>().connect<&SceneGame::onMouse>(this);
    this->m_dispatcher->sink<WindowKeyEvent>().connect<&SceneGame::onKeyboard>(this);

    this->m_card_hand = std::make_unique<CardHand>(this->m_renderer, this->m_assets, this->m_audio);
    this->m_timer_gui = std::make_unique<TimerGUI>(this->m_renderer, this->m_assets, this->m_audio);
    this->m_battle =
        std::make_shared<battle::BattleSystem>(*this->m_dispatcher, this->m_assets, this->m_audio);
    this->m_lunar_cycle_gui = std::make_unique<LunarCycleGUI>(this->m_renderer, this->m_assets,
                                                              this->m_audio, this->m_battle);
    this->m_player =
        std::make_unique<Player>(this->m_renderer, this->m_assets, this->m_audio, this->m_battle);

    int w, h, c;
    auto pixel = this->m_assets->loadImage("assets/images/gameplay_bg.png", w, h, c);
    this->m_renderer->createTexture("gameplay_bg", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/gui/bottom_bar.png", w, h, c);
    this->m_renderer->createTexture("bottom_bar", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    pixel = this->m_assets->loadImage("assets/images/gui/button.png", w, h, c);
    this->m_renderer->createTexture("button", pixel, w, h);
    this->m_assets->unLoadImage(pixel);

    int channels, sample_rate;
    short *data;
    int sim =
        this->m_assets->loadAudio("assets/audio/gameplay_bgm.ogg", channels, sample_rate, data);
    this->m_audio->load("gameplay", channels, sim, sample_rate, data);
    this->m_audio->set_bgm_fade_gain(0);
    this->m_audio->fade_bgm(1.0f, 5.0f);
    this->m_audio->play_bgm("gameplay", true, 0.1);

    this->m_player->onEnter();
    this->m_card_hand->onEnter();
    this->m_timer_gui->onEnter();
    this->m_lunar_cycle_gui->onEnter();

    this->m_overlay = OverlayState::NONE;
    this->m_play_time = 0.0f;
    this->m_level_reached = 1;
    this->m_prev_phase = battle::CombatPhase::IDLE;

    this->m_battle->startBattle(1);
}

void SceneGame::onUpdate(double deltaTime)
{
    const float fdt = static_cast<float>(deltaTime);
    this->m_arrow_pulse += fdt;
    this->m_enemy_anim_time += fdt;

    // ── ESC: toggle pause (only during player turn) ───────────────────────────
    if (this->m_key_escape) {
        if (this->m_overlay == OverlayState::PAUSE) {
            // Resume
            this->m_overlay = OverlayState::NONE;
            if (this->m_battle &&
                this->m_battle->getContext().phase == battle::CombatPhase::PLAYER_TURN)
                this->m_timer_gui->startTurn(10.0f);
        } else if (this->m_overlay == OverlayState::NONE && this->m_battle &&
                   this->m_battle->getContext().phase == battle::CombatPhase::PLAYER_TURN) {
            this->m_overlay = OverlayState::PAUSE;
            this->m_timer_gui->stopTurn();
        }
        this->m_key_escape = false;
    }

    // ── Overlay active: handle button clicks then stop ────────────────────────
    if (this->m_overlay != OverlayState::NONE) {
        // Must match drawOverlay constants exactly
        constexpr float PH = 360.0f, SH = 720.0f;
        constexpr float PY = (SH - PH) * 0.5f; // 180
        constexpr float BW = 220.0f, BH = 50.0f;
        constexpr float BCX = 640.0f - BW * 0.5f;

        auto inBtn = [&](float y) {
            return this->m_mouse_clicked && this->m_mouse_x >= BCX && this->m_mouse_x <= BCX + BW &&
                   this->m_mouse_y >= y && this->m_mouse_y <= y + BH;
        };

        if (this->m_overlay == OverlayState::PAUSE) {
            if (inBtn(PY + 170.0f)) { // Resume
                this->m_overlay = OverlayState::NONE;
                if (this->m_battle &&
                    this->m_battle->getContext().phase == battle::CombatPhase::PLAYER_TURN)
                    this->m_timer_gui->startTurn(10.0f);
            } else if (inBtn(PY + 240.0f)) { // Main Menu
                this->m_dispatcher->trigger<SceneEvent>({"menu"});
            }
        } else {
            // GAME_OVER and VICTORY share the same layout
            if (inBtn(PY + 210.0f)) { // Restart / Play Again
                this->m_play_time = 0.0f;
                this->m_dispatcher->trigger<SceneEvent>({"gameplay"});
            } else if (inBtn(PY + 280.0f)) { // Main Menu
                this->m_dispatcher->trigger<SceneEvent>({"menu"});
            }
        }

        this->m_mouse_clicked = false;
        this->m_right_clicked = false;
        this->m_key_play = false;
        this->m_key_end_turn = false;
        return;
    }

    // ── Track play time ───────────────────────────────────────────────────────
    if (this->m_battle && this->m_battle->isActive())
        this->m_play_time += fdt;

    // ── Determine if selected card targets self ───────────────────────────────
    this->m_self_targeting = false;
    this->m_hovered_enemy = entt::null;
    this->m_hovered_self = false;

    int cur_sel = this->m_card_hand->getSelectedSlot();
    if (cur_sel >= 0 && this->m_battle) {
        entt::entity pe = this->m_battle->getPlayer();
        if (pe != entt::null) {
            const auto &bhand = this->m_battle->getRegistry().get<battle::HandComp>(pe);
            if (cur_sel < bhand.count) {
                int32_t cid = bhand.slots[static_cast<size_t>(cur_sel)];
                const CardInfo *info = this->m_battle->getCardDb().findByIndex(cid);
                if (info)
                    this->m_self_targeting = battle::cardTargetsSelf(*info);
            }
        }
    }

    // ── Hover detection (enemy -or- player depending on card type) ────────────
    if (cur_sel >= 0 && this->m_battle) {
        if (this->m_self_targeting) {
            // Player hit area (generous, around sprite at 180,280 size 64×64)
            constexpr float PX1 = 148.0f, PX2 = 278.0f;
            constexpr float PY1 = 258.0f, PY2 = 368.0f;
            this->m_hovered_self = (this->m_mouse_x >= PX1 && this->m_mouse_x <= PX2 &&
                                    this->m_mouse_y >= PY1 && this->m_mouse_y <= PY2);
        } else {
            for (const auto &slot : this->m_enemy_slots) {
                constexpr float PAD = 12.0f;
                if (this->m_mouse_x >= slot.x - PAD && this->m_mouse_x <= slot.x + slot.w + PAD &&
                    this->m_mouse_y >= slot.y - PAD && this->m_mouse_y <= slot.y + slot.h + PAD) {
                    this->m_hovered_enemy = slot.entity;
                    break;
                }
            }
        }
    }

    // ── Click: self-target card on player ─────────────────────────────────────
    if (this->m_mouse_clicked && this->m_hovered_self && this->m_battle) {
        if (this->m_battle->getContext().phase == battle::CombatPhase::PLAYER_TURN) {
            int sel = this->m_card_hand->getSelectedSlot();
            if (sel >= 0) {
                this->m_card_hand->clearSelection();
                this->m_battle->playerPlayCard(sel); // entt::null → self only
                this->m_hovered_self = false;
                this->m_mouse_clicked = false;
            }
        }
    }

    // ── Click: enemy-target card on enemy ─────────────────────────────────────
    if (this->m_mouse_clicked && this->m_hovered_enemy != entt::null && this->m_battle) {
        if (this->m_battle->getContext().phase == battle::CombatPhase::PLAYER_TURN) {
            int sel = this->m_card_hand->getSelectedSlot();
            if (sel >= 0) {
                this->m_card_hand->clearSelection();
                this->m_battle->playerPlayCard(sel, this->m_hovered_enemy);
                this->m_hovered_enemy = entt::null;
                this->m_mouse_clicked = false;
            }
        }
    }

    // ── CardHand update (hover, selection, right-click discard) ───────────────
    this->m_player->onUpdate(deltaTime);
    this->m_card_hand->onUpdate(deltaTime, this->m_mouse_x, this->m_mouse_y, this->m_mouse_clicked,
                                this->m_right_clicked);
    this->m_timer_gui->onUpdate(deltaTime);
    this->m_lunar_cycle_gui->onUpdate(deltaTime);

    // ── Battle input ──────────────────────────────────────────────────────────
    if (this->m_battle) {
        const auto &ctx = this->m_battle->getContext();

        if (ctx.phase == battle::CombatPhase::PLAYER_TURN) {
            // Right-click discard
            int ds = this->m_card_hand->consumeDiscardedSlot();
            if (ds >= 0)
                this->m_battle->playerDiscard(ds);

            // Space/Enter → play selected card (self-target or at hovered/first enemy)
            if (this->m_key_play) {
                int sel = this->m_card_hand->getSelectedSlot();
                if (sel >= 0) {
                    this->m_card_hand->clearSelection();
                    if (this->m_self_targeting)
                        this->m_battle->playerPlayCard(sel);
                    else
                        this->m_battle->playerPlayCard(sel, this->m_hovered_enemy);
                    this->m_hovered_self = false;
                    this->m_hovered_enemy = entt::null;
                }
            }

            // E or End Turn button click
            bool btn_click = this->m_mouse_clicked && this->m_mouse_x >= 1050.0f &&
                             this->m_mouse_x <= 1210.0f && this->m_mouse_y >= 550.0f &&
                             this->m_mouse_y <= 600.0f;

            if (this->m_key_end_turn || btn_click) {
                this->m_timer_gui->stopTurn();
                this->m_card_hand->clearSelection();
                this->m_battle->playerEndTurn();
            }

            // Turn timer expired → auto end turn
            if (this->m_timer_gui->isExpired()) {
                this->m_card_hand->clearSelection();
                this->m_battle->playerEndTurn();
            }
        }

        this->m_battle->update(deltaTime);

        // ── Phase transition → manage turn timer + overlays ──────────────────
        const auto &ctx2 = this->m_battle->getContext();
        if (ctx2.phase != this->m_prev_phase) {
            if (ctx2.phase == battle::CombatPhase::PLAYER_TURN) {
                this->m_timer_gui->startTurn(10.0f);
            } else {
                this->m_timer_gui->stopTurn();
                if (ctx2.phase == battle::CombatPhase::DEFEAT) {
                    this->m_level_reached = ctx2.turn_number;
                    this->m_overlay = OverlayState::GAME_OVER;
                } else if (ctx2.phase == battle::CombatPhase::IDLE &&
                           this->m_prev_phase == battle::CombatPhase::VICTORY) {
                    // All levels cleared — final boss defeated
                    this->m_level_reached = ctx2.turn_number;
                    this->m_overlay = OverlayState::VICTORY;
                }
            }
            this->m_prev_phase = ctx2.phase;
        }

        // Sync visual hand with battle hand state
        entt::entity player = this->m_battle->getPlayer();
        if (player != entt::null) {
            const auto &hand = this->m_battle->getRegistry().get<battle::HandComp>(player);
            this->m_card_hand->syncWithBattle(hand);
        }
    }

    this->m_mouse_clicked = false;
    this->m_right_clicked = false;
    this->m_key_play = false;
    this->m_key_end_turn = false;
}

void SceneGame::onDraw()
{
    this->m_renderer->oxDrawSprite(0, 0, 1280, 720, "gameplay_bg", {255, 255, 255, 100});
    this->m_renderer->oxDrawSprite(0, 0, 1280, 720, "bottom_bar", {255, 255, 255, 255}, 2);
    this->m_player->onDraw();

    if (this->m_battle) {
        this->drawEnemies();        // populates m_enemy_slots
        this->drawTargetingArrow(); // uses m_enemy_slots
        this->drawHUD();
    }

    this->m_card_hand->onDraw();
    this->m_timer_gui->onDraw();
    this->m_lunar_cycle_gui->onDraw();
    this->drawOverlay();
}

void SceneGame::onExit()
{
    LOG_TRACE("[SceneGame] Exit");
    this->m_dispatcher->sink<WindowMouseEvent>().disconnect(this);
    this->m_dispatcher->sink<WindowKeyEvent>().disconnect(this);
    this->m_audio->stop_bgm();
    this->m_renderer->freeTexture("gameplay_bg");
    this->m_renderer->freeTexture("botoom_bar");
    this->m_renderer->freeTexture("button");
    this->m_audio->unload("gameplay_bg");

    if (this->m_battle) {
        this->m_battle->shutdown();
        this->m_battle.reset();
    }
}

// ============================================================
// Enemy rendering helpers
// ============================================================

static Color enemyTint(battle::EnemyType type)
{
    switch (type) {
    case battle::EnemyType::SKELETON:
        return {210, 215, 255, 255};
    case battle::EnemyType::GOBLIN:
        return {130, 230, 130, 255};
    case battle::EnemyType::TROLL:
        return {200, 145, 90, 255};
    case battle::EnemyType::ARCHER:
        return {220, 210, 90, 255};
    case battle::EnemyType::DARK_KNIGHT:
        return {140, 130, 210, 255};
    case battle::EnemyType::NECROMANCER:
        return {210, 90, 210, 255};
    case battle::EnemyType::BOSS_DRAGON:
        return {255, 80, 80, 255};
    case battle::EnemyType::BOSS_LICH:
        return {170, 80, 255, 255};
    default:
        return Color::White();
    }
}

void SceneGame::drawEnemies()
{
    this->m_enemy_slots.clear();
    auto &reg = this->m_battle->getRegistry();

    std::vector<entt::entity> enemies;
    for (auto e : reg.view<battle::EnemyTag>(entt::exclude<battle::DeadTag>))
        enemies.push_back(e);

    if (enemies.empty())
        return;

    constexpr float BASE_Y = 220.0f;
    constexpr float BASE_W = 80.0f;
    constexpr float BASE_H = 80.0f;
    constexpr float SPACING = 170.0f;

    int n = static_cast<int>(enemies.size());
    float sx = 900.0f - (n - 1) * SPACING * 0.5f;

    // Idle animation: 4 frames across top row of player sheet, flipped
    int frame = static_cast<int>(this->m_enemy_anim_time / 0.12f) % 4;
    float u0_flip = (frame + 1) * 0.25f;
    float u1_flip = frame * 0.25f;
    constexpr float V0 = 0.0f;
    constexpr float V1 = 0.5f;

    for (int i = 0; i < n; ++i) {
        entt::entity e = enemies[i];
        bool is_boss = false;
        if (auto *et = reg.try_get<battle::EnemyTypeComp>(e))
            is_boss = (et->type == battle::EnemyType::BOSS_DRAGON ||
                       et->type == battle::EnemyType::BOSS_LICH);

        float w = is_boss ? BASE_W * 1.5f : BASE_W;
        float h = is_boss ? BASE_H * 1.5f : BASE_H;
        float ex = sx + i * SPACING;
        float ey = BASE_Y;

        this->m_enemy_slots.push_back({e, ex, ey, w, h});

        Color tint = Color::White();
        if (auto *et = reg.try_get<battle::EnemyTypeComp>(e))
            tint = enemyTint(et->type);

        // ── Hover highlight (drawn BEFORE sprite so it appears behind) ────────
        bool hovered = (e == this->m_hovered_enemy);
        if (hovered) {
            float pulse = 1.0f + 0.18f * std::sin(this->m_arrow_pulse * 6.0f);
            float hw = w * 0.5f * pulse + 10.0f;
            float hh = h * 0.5f * pulse + 10.0f;
            uint8_t ha = static_cast<uint8_t>(80 + 60 * std::sin(this->m_arrow_pulse * 4.0f));
            this->m_renderer->oxDrawRectangle(ex + w * 0.5f, ey + h * 0.5f, hw * 2.0f, hh * 2.0f,
                                              {255, 60, 60, ha}, 0, 0.0f, 0.5f, 0.5f);
        }

        // ── Enemy sprite ──────────────────────────────────────────────────────
        this->m_renderer->oxDrawSpriteSheet(ex, ey, w, h, "player", u0_flip, V0, u1_flip, V1, tint,
                                            1);

        // ── Name ─────────────────────────────────────────────────────────────
        if (auto *nm = reg.try_get<battle::NameComp>(e)) {
            float tw = this->m_renderer->measureText(nm->data, 12);
            this->m_renderer->oxDrawText(ex + w * 0.5f - tw * 0.5f, ey - 42.0f, nm->data, 12,
                                         Color::White(), TextEffect::Outline(Color::Black()), 2);
        }

        // ── Intent ────────────────────────────────────────────────────────────
        if (auto *intent = reg.try_get<battle::IntentComp>(e)) {
            std::string txt;
            switch (intent->type) {
            case battle::IntentType::ATTACK:
                txt = std::format("ATK {}", intent->damage);
                break;
            case battle::IntentType::DEFEND:
                txt = std::format("BLK {}", intent->block);
                break;
            case battle::IntentType::BUFF:
                txt = "BUFF";
                break;
            case battle::IntentType::DEBUFF:
                txt = "DEBUFF";
                break;
            case battle::IntentType::SPECIAL:
                txt = "SPECIAL";
                break;
            }
            float tw = this->m_renderer->measureText(txt.c_str(), 13);
            this->m_renderer->oxDrawText(ex + w * 0.5f - tw * 0.5f, ey - 22.0f, txt.c_str(), 13,
                                         {255, 210, 80, 255}, TextEffect::Outline(Color::Black()),
                                         2);
        }

        // ── HP bar ────────────────────────────────────────────────────────────
        if (auto *hp = reg.try_get<battle::HealthComp>(e)) {
            float bar_y = ey + h + 6.0f;
            this->m_renderer->oxDrawRectangle(ex, bar_y, w, 10.0f, {70, 10, 10, 200}, 2);
            float fill = w * hp->ratio();
            if (fill > 0)
                this->m_renderer->oxDrawRectangle(ex, bar_y, fill, 10.0f, {220, 50, 50, 255}, 2);
            std::string t = std::format("{}/{}", hp->current, hp->max);
            this->m_renderer->oxDrawText(ex, bar_y + 13.0f, t.c_str(), 11, Color::White(),
                                         TextEffect::Outline(Color::Black()), 2);
        }

        // ── Block badge ───────────────────────────────────────────────────────
        if (auto *bl = reg.try_get<battle::BlockComp>(e); bl && bl->amount > 0) {
            std::string t = std::format("BLK {}", bl->amount);
            this->m_renderer->oxDrawText(ex, ey - 58.0f, t.c_str(), 11, {100, 180, 255, 255},
                                         TextEffect::Outline(Color::Black()), 2);
        }
    }
}

// ============================================================
// Targeting arrow helpers
// ============================================================

// Draw a downward arrow (↓) above a given screen point.
// col = main color, glow = halo color, pulse = time accumulator.
void SceneGame::drawArrowAt(float cx, float top_y, Color col, Color glow)
{
    float bob = std::sin(this->m_arrow_pulse * 5.5f) * 5.0f;
    float tip_y = top_y - 14.0f + bob;

    constexpr float THICK = 5.0f;
    constexpr float SHAFT = 16.0f;
    constexpr float WX = 12.0f;
    constexpr float WY = 10.0f;

    float shaft_cy = tip_y - WY - SHAFT * 0.5f;
    float wlen = std::sqrt(WX * WX + WY * WY);
    float lw_angle = std::atan2(-WY, -WX);
    float rw_angle = std::atan2(-WY, WX);
    float lw_cx = cx - WX * 0.5f;
    float lw_cy = tip_y - WY * 0.5f;
    float rw_cx = cx + WX * 0.5f;
    float rw_cy = tip_y - WY * 0.5f;

    // Glow layer
    this->m_renderer->oxDrawRectangle(cx, shaft_cy, THICK + 6, SHAFT + 4, glow, 3, 0.0f, 0.5f,
                                      0.5f);
    this->m_renderer->oxDrawRectangle(lw_cx, lw_cy, wlen + 4, THICK + 2, glow, 3, lw_angle, 0.5f,
                                      0.5f);
    this->m_renderer->oxDrawRectangle(rw_cx, rw_cy, wlen + 4, THICK + 2, glow, 3, rw_angle, 0.5f,
                                      0.5f);
    // Arrow
    this->m_renderer->oxDrawRectangle(cx, shaft_cy, THICK, SHAFT, col, 4, 0.0f, 0.5f, 0.5f);
    this->m_renderer->oxDrawRectangle(lw_cx, lw_cy, wlen, THICK, col, 4, lw_angle, 0.5f, 0.5f);
    this->m_renderer->oxDrawRectangle(rw_cx, rw_cy, wlen, THICK, col, 4, rw_angle, 0.5f, 0.5f);
    // Dot at tip
    float dot_r = 5.0f + 2.5f * std::sin(this->m_arrow_pulse * 7.0f);
    this->m_renderer->oxDrawCircle(cx, tip_y, dot_r, col, 4);
}

void SceneGame::drawTargetingArrow()
{
    if (this->m_card_hand->getSelectedSlot() < 0)
        return;

    uint8_t a = static_cast<uint8_t>(210 + 45 * std::sin(this->m_arrow_pulse * 4.0f));

    if (this->m_self_targeting) {
        // ── Self-targeting card → indicator above player character ────────────
        // Player sprite
        float PCX = this->m_player->getX();
        float PTY = this->m_player->getY();

        Color col = {60, 160, 255, a};
        Color glow = {100, 200, 255, static_cast<uint8_t>(a * 50 / 255)};
        drawArrowAt(PCX, PTY, col, glow);

        // Highlight around player when hovering
        if (this->m_hovered_self) {
            float pulse = 1.0f + 0.15f * std::sin(this->m_arrow_pulse * 6.0f);
            float hw = 36.0f * pulse, hh = 36.0f * pulse;
            uint8_t ha = static_cast<uint8_t>(70 + 60 * std::sin(this->m_arrow_pulse * 4.0f));
            this->m_renderer->oxDrawRectangle(PCX, 280.0f + 32.0f, hw * 2.0f, hh * 2.0f,
                                              {60, 160, 255, ha}, 0, 0.0f, 0.5f, 0.5f);
        }

    } else if (this->m_hovered_enemy != entt::null) {
        // ── Enemy-targeting card → indicator above hovered enemy ──────────────
        float enemy_cx = 0.0f;
        float enemy_top = 0.0f;
        bool found = false;
        for (const auto &slot : this->m_enemy_slots) {
            if (slot.entity == this->m_hovered_enemy) {
                enemy_cx = slot.x + slot.w * 0.5f;
                enemy_top = slot.y;
                found = true;
                break;
            }
        }
        if (!found)
            return;

        Color col = {255, 50, 50, a};
        Color glow = {255, 140, 60, static_cast<uint8_t>(a * 55 / 255)};
        drawArrowAt(enemy_cx, enemy_top, col, glow);
    }
}

// ============================================================
// HUD
// ============================================================

void SceneGame::drawHUD()
{
    auto &reg = this->m_battle->getRegistry();
    entt::entity player = this->m_battle->getPlayer();
    const auto &ctx = this->m_battle->getContext();

    if (player != entt::null) {
        constexpr float HX = 24.0f;

        // Block
        if (auto *bl = reg.try_get<battle::BlockComp>(player); bl && bl->amount > 0) {
            std::string t = std::format("BLK  {}", bl->amount);
            this->m_renderer->oxDrawText(HX, 682.0f, t.c_str(), 13, {100, 180, 255, 255},
                                         TextEffect::Outline(Color::Black()), 3);
        }

        // Energy
        // if (auto *en = reg.try_get<battle::EnergyComp>(player)) {
        //     std::string t = std::format("NRG  {}/{}", en->current, en->max);
        //     this->m_renderer->oxDrawText(260.0f, 646.0f, t.c_str(), 13, {255, 215, 80, 255},
        //                                  TextEffect::Outline(Color::Black()), 3);
        // }

        // Level
        if (auto *lvl = reg.try_get<battle::LevelComp>(player)) {
            std::string t = std::format("Level {}", lvl->current);
            this->m_renderer->oxDrawText(HX, 29.0f, t.c_str(), 16, Color::White(),
                                         TextEffect::Outline(Color::Black()), 3);
        }
    }

    // Phase banner
    const char *phase_txt = nullptr;
    Color phase_col = Color::White();
    switch (ctx.phase) {
    case battle::CombatPhase::PLAYER_TURN:
        phase_txt = "Your Turn";
        phase_col = {200, 240, 255, 255};
        break;
    case battle::CombatPhase::ENEMY_TURN:
        phase_txt = "Enemy Turn";
        phase_col = {255, 180, 100, 255};
        break;
    case battle::CombatPhase::VICTORY:
        phase_txt = "VICTORY!";
        phase_col = {120, 255, 120, 255};
        break;
    case battle::CombatPhase::DEFEAT:
        phase_txt = "DEFEAT";
        phase_col = {255, 80, 80, 255};
        break;
    case battle::CombatPhase::BOSS_INTRO:
        phase_txt = "BOSS ENCOUNTER!";
        phase_col = {255, 100, 100, 255};
        break;
    default:
        break;
    }
    if (phase_txt) {
        float tw = this->m_renderer->measureText(phase_txt, 22);
        this->m_renderer->oxDrawText(640.0f - tw * 0.5f, 29.0f, phase_txt, 22, phase_col,
                                     TextEffect::Outline(Color::Black()), 3);
    }

    // Targeting hint (when card is selected but no enemy hovered)
    bool card_selected = (this->m_card_hand->getSelectedSlot() >= 0);
    bool has_enemies = !this->m_enemy_slots.empty();
    if (card_selected && has_enemies && this->m_hovered_enemy == entt::null &&
        ctx.phase == battle::CombatPhase::PLAYER_TURN) {
        const char *hint = "Click enemy to use card";
        float tw = this->m_renderer->measureText(hint, 14);
        this->m_renderer->oxDrawText(640.0f - tw * 0.5f, 84.0f, hint, 14, {255, 220, 100, 255},
                                     TextEffect::Outline(Color::Black()), 3);
    }

    // End Turn button
    if (ctx.phase == battle::CombatPhase::PLAYER_TURN) {
        bool hover_btn = (this->m_mouse_x >= 1050.0f && this->m_mouse_x <= 1210.0f &&
                          this->m_mouse_y >= 550.0f && this->m_mouse_y <= 600.0f);
        Color btn_col = hover_btn ? Color{80, 110, 200, 255} : Color{50, 60, 110, 230};
        this->m_renderer->oxDrawSprite(1050.0f, 550.0f, 160.0f, 50.0f, "button", Color::White(), 2);
        float tw = this->m_renderer->measureText("END TURN [E]", 14);
        this->m_renderer->oxDrawText(1040.0f + (tw / 2), 550.0f + 25, "END TURN [E]", 14,
                                     Color::White(), TextEffect::Outline(Color::Black()), 3);
    }

    // Turn counter
    if (ctx.phase != battle::CombatPhase::IDLE) {
        std::string tt = std::format("Turn {}", ctx.turn_number + 1);
        float tw = this->m_renderer->measureText(tt.c_str(), 16);
        this->m_renderer->oxDrawText(1280.0f - tw - 30.0f, 29.0f, tt.c_str(), 16,
                                     {255, 180, 100, 255}, TextEffect::Outline(Color::Black()), 3);
    }
}

// ============================================================
// Input
// ============================================================
// Overlay helpers
// ============================================================

static std::string fmtTime(float secs)
{
    int s = static_cast<int>(secs);
    return std::format("{:02d}:{:02d}", s / 60, s % 60);
}

bool SceneGame::overlayBtn(const char *text, float x, float y, float w, float h)
{
    bool hov = m_mouse_x >= x && m_mouse_x <= x + w && m_mouse_y >= y && m_mouse_y <= y + h;
    Color bg = hov ? Color{80, 130, 230, 245} : Color{30, 50, 110, 210};
    Color bdr = hov ? Color{160, 200, 255, 255} : Color{70, 100, 180, 200};
    m_renderer->oxDrawRectangle(x - 2, y - 2, w + 4, h + 4, bdr, 22, 0, 0, 0);
    m_renderer->oxDrawRectangle(x, y, w, h, bg, 23, 0, 0, 0);
    float fs = 20.0f;
    float tw = m_renderer->measureText(text, fs);
    float lh = m_renderer->fontMetrics().lineHeight * fs;
    m_renderer->oxDrawText(x + w * 0.5f - tw * 0.5f, y + h - lh * 0.5f, text, fs, Color::White(),
                           TextEffect::None(), 24);
    return false; // clicks handled in onUpdate
}

void SceneGame::drawOverlay()
{
    if (m_overlay == OverlayState::NONE)
        return;

    constexpr float SW = 1280.0f, SH = 720.0f;
    constexpr float PW = 520.0f, PH = 360.0f;
    constexpr float PX = (SW - PW) * 0.5f, PY = (SH - PH) * 0.5f;
    constexpr float CX = SW * 0.5f;
    constexpr float BW = 220.0f, BH = 50.0f;
    constexpr float BCX = CX - BW * 0.5f;

    // Dim screen
    m_renderer->oxDrawRectangle(0, 0, SW, SH, Color{0, 0, 0, 160}, 19, 0, 0, 0);

    // Panel border + fill
    m_renderer->oxDrawRectangle(PX - 3, PY - 3, PW + 6, PH + 6, Color{70, 100, 190, 220}, 20, 0, 0,
                                0);
    m_renderer->oxDrawRectangle(PX, PY, PW, PH, Color{10, 15, 45, 240}, 21, 0, 0, 0);

    if (m_overlay == OverlayState::PAUSE) {
        // Title
        const char *title = "PAUSED";
        float tw = m_renderer->measureText(title, 48.0f);
        m_renderer->oxDrawText(CX - tw * 0.5f, PY + 50.0f, title, 48.0f, Color{180, 210, 255, 255},
                               TextEffect::Glow(Color{100, 160, 255, 200}, 0.4f, 2.0f), 22);

        const char *hint = "Press ESC to resume";
        float hw = m_renderer->measureText(hint, 16.0f);
        m_renderer->oxDrawText(CX - hw * 0.5f, PY + 115.0f, hint, 16.0f, Color{140, 160, 200, 200},
                               TextEffect::None(), 22);

        overlayBtn("Resume", BCX, PY + 170.0f, BW, BH);
        overlayBtn("Main Menu", BCX, PY + 240.0f, BW, BH);

    } else if (m_overlay == OverlayState::GAME_OVER) {
        const char *title = "GAME OVER";
        float tw = m_renderer->measureText(title, 44.0f);
        m_renderer->oxDrawText(CX - tw * 0.5f, PY + 40.0f, title, 44.0f, Color{255, 80, 80, 255},
                               TextEffect::Glow(Color{200, 40, 40, 200}, 0.5f, 2.5f), 22);

        auto lvlStr = std::format("Level reached : {}", m_level_reached);
        auto timeStr = std::format("Time played   : {}", fmtTime(m_play_time));
        float ls = 20.0f;
        float lw1 = m_renderer->measureText(lvlStr.c_str(), ls);
        float lw2 = m_renderer->measureText(timeStr.c_str(), ls);
        m_renderer->oxDrawText(CX - lw1 * 0.5f, PY + 115.0f, lvlStr.c_str(), ls,
                               Color{200, 220, 255, 220}, TextEffect::None(), 22);
        m_renderer->oxDrawText(CX - lw2 * 0.5f, PY + 145.0f, timeStr.c_str(), ls,
                               Color{200, 220, 255, 220}, TextEffect::None(), 22);

        overlayBtn("Restart", BCX, PY + 210.0f, BW, BH);
        overlayBtn("Main Menu", BCX, PY + 280.0f, BW, BH);

    } else if (m_overlay == OverlayState::VICTORY) {
        const char *title = "VICTORY!";
        float tw = m_renderer->measureText(title, 48.0f);
        m_renderer->oxDrawText(CX - tw * 0.5f, PY + 40.0f, title, 48.0f, Color{255, 220, 60, 255},
                               TextEffect::Glow(Color{200, 160, 0, 200}, 0.5f, 2.5f), 22);

        auto lvlStr = std::format("Level reached : {}", m_level_reached);
        auto timeStr = std::format("Time played   : {}", fmtTime(m_play_time));
        float ls = 20.0f;
        float lw1 = m_renderer->measureText(lvlStr.c_str(), ls);
        float lw2 = m_renderer->measureText(timeStr.c_str(), ls);
        m_renderer->oxDrawText(CX - lw1 * 0.5f, PY + 115.0f, lvlStr.c_str(), ls,
                               Color{220, 240, 180, 220}, TextEffect::None(), 22);
        m_renderer->oxDrawText(CX - lw2 * 0.5f, PY + 145.0f, timeStr.c_str(), ls,
                               Color{220, 240, 180, 220}, TextEffect::None(), 22);

        overlayBtn("Play Again", BCX, PY + 210.0f, BW, BH);
        overlayBtn("Main Menu", BCX, PY + 280.0f, BW, BH);
    }
}

// ============================================================

void SceneGame::onMouse(const WindowMouseEvent &event)
{
    this->m_mouse_x = float(event.mouseX);
    this->m_mouse_y = float(event.mouseY);
    if (event.click == 1) {
        if (!this->m_mouse_held)
            this->m_mouse_clicked = true;
        this->m_mouse_held = true;
    } else if (event.click == 3) {
        this->m_right_clicked = true;
    } else {
        this->m_mouse_held = false;
    }
}

void SceneGame::onKeyboard(const WindowKeyEvent &event)
{
    if (event.key == SDL_SCANCODE_SPACE || event.key == SDL_SCANCODE_RETURN)
        this->m_key_play = true;
    else if (event.key == SDL_SCANCODE_E)
        this->m_key_end_turn = true;
    else if (event.key == SDL_SCANCODE_ESCAPE)
        this->m_key_escape = true;
}
