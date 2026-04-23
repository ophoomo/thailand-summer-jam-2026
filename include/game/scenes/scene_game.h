#ifndef B21E8FB9_EE83_4642_9675_594206EBD3B8
#define B21E8FB9_EE83_4642_9675_594206EBD3B8

#include "engine/core/scene.h"
#include "game/battle/battle_system.h"
#include "game/battle/combat_state.h"
#include "game/cards/hand.h"
#include "game/characters/player.h"
#include "game/gui/lunar_cycle_gui.h"
#include "game/gui/timer_gui.h"
#include "game/particles/background_particles.h"
#include "renderer/renderer_interface.h"
#include <entt/entt.hpp>
#include <memory>
#include <vector>

class SceneGame : public Scene
{
  public:
    SceneGame(std::shared_ptr<entt::dispatcher> dispatcher, std::shared_ptr<OxRenderer> renderer,
              std::shared_ptr<AssetsInterface> assets, std::shared_ptr<AudioInterface> audio)
        : Scene(dispatcher, renderer, assets, audio)
    {
    }
    ~SceneGame() = default;

    void onEnter() override;
    void onDraw() override;
    void onUpdate(double deltaTime) override;
    void onExit() override;

  private:
    void onMouse(const WindowMouseEvent &event);
    void onKeyboard(const WindowKeyEvent &event);

    void drawEnemies();
    void drawHUD();
    void drawTargetingArrow();
    void drawArrowAt(float cx, float top_y, Color col, Color glow);
    void drawOverlay();
    bool overlayBtn(const char *text, float x, float y, float w, float h);

    // ── UI objects ────────────────────────────────────────────────────────────
    std::unique_ptr<Player> m_player;
    std::unique_ptr<CardHand> m_card_hand;
    std::unique_ptr<TimerGUI> m_timer_gui;
    std::unique_ptr<LunarCycleGUI> m_lunar_cycle_gui;
    std::shared_ptr<battle::BattleSystem> m_battle;
    std::unique_ptr<Animator> m_enemy_animator;
    std::unique_ptr<BackgroundParticleEmitter> m_bg_particle;

    // ── Enemy render cache (populated in drawEnemies each frame) ──────────────
    struct EnemyRenderSlot
    {
        entt::entity entity;
        float x, y, w, h;
    };
    std::vector<EnemyRenderSlot> m_enemy_slots;

    // ── Targeting state ────────────────────────────────────────────────────────
    entt::entity m_hovered_enemy{entt::null};
    bool m_hovered_self = false;   // mouse over player area (for self-target cards)
    bool m_self_targeting = false; // selected card targets self (recomputed each frame)
    float m_arrow_pulse = 0.0f;
    float m_enemy_anim_time = 0.0f;

    // ── Mouse / keyboard state ─────────────────────────────────────────────────
    float m_mouse_x = 0;
    float m_mouse_y = 0;
    bool m_mouse_clicked = false;
    bool m_mouse_held = false;
    bool m_right_clicked = false;

    bool m_key_play = false;
    bool m_key_end_turn = false;
    bool m_key_escape = false;
    bool m_button_hover_sound = false;

    battle::CombatPhase m_prev_phase{battle::CombatPhase::IDLE};

    // ── Overlay (pause / game-over / victory) ─────────────────────────────────
    enum class OverlayState : uint8_t { NONE, PAUSE, GAME_OVER, VICTORY };
    OverlayState m_overlay{OverlayState::NONE};
    float m_play_time{0.0f};
    int32_t m_level_reached{1};
};

#endif /* B21E8FB9_EE83_4642_9675_594206EBD3B8 */
