#ifndef HELIORA_SCENE_STORY_H
#define HELIORA_SCENE_STORY_H

#include "engine/core/scene.h"
#include "ui/ui_system.h"
#include <string>
#include <vector>

// ── Per-slide timing ──────────────────────────────────────────────────────────
static constexpr float STORY_FADE_IN_SEC  = 1.2f;
static constexpr float STORY_HOLD_SEC     = 2.8f;
static constexpr float STORY_FADE_OUT_SEC = 1.0f;

// ── Outro (splash_scene) timing ───────────────────────────────────────────────
static constexpr float OUTRO_FADE_IN_SEC  = 1.5f;
static constexpr float OUTRO_HOLD_SEC     = 2.0f;
static constexpr float OUTRO_FADE_OUT_SEC = 1.5f;
static constexpr float OUTRO_TOTAL_SEC    =
    OUTRO_FADE_IN_SEC + OUTRO_HOLD_SEC + OUTRO_FADE_OUT_SEC;

class SceneStory : public Scene
{
  public:
    SceneStory(std::shared_ptr<entt::dispatcher> dispatcher,
               std::shared_ptr<OxRenderer>       renderer,
               std::shared_ptr<AssetsInterface>  assets,
               std::shared_ptr<AudioInterface>   audio,
               std::shared_ptr<CursorUI>         cursor)
        : Scene(dispatcher, renderer, assets, audio, cursor)
    {}
    ~SceneStory() = default;

    void onEnter()           override;
    void onDraw()            override;
    void onUpdate(double dt) override;
    void onExit()            override;

  private:
    // ── UISystem (owns all widgets) ───────────────────────────────────────────
    std::unique_ptr<UISystem> m_ui;

    // ── Slides: each inner vector = one slide (multi-line) ───────────────────
    std::vector<std::vector<std::string>> m_slides;
    int m_slideIndex = 0;

    // ── State machine ─────────────────────────────────────────────────────────
    enum class State { FadeIn, Hold, FadeOut, Outro, Done };
    State m_state  = State::FadeIn;
    float m_timer  = 0.0f;

    // ── Outro ─────────────────────────────────────────────────────────────────
    float m_outroTimer = 0.0f;
    bool  m_outroDone  = false;

    // ── Input ─────────────────────────────────────────────────────────────────
    float m_mouseX     = 0.0f;
    float m_mouseY     = 0.0f;
    bool  m_mouseClick = false;   // single-frame
    bool  m_mouseHeld  = false;
    bool  m_skipQueued = false;

    void onMouse(const WindowMouseEvent& e);

    // ── Helpers ───────────────────────────────────────────────────────────────
    void buildSlides();
    void buildStoryWidgets();   // (re)create label widgets for current slide
    void advanceSlide();

    void updateStory(float dt);
    void updateOutro(float dt);

    void applyAlpha(float textA, float hintA);  // update widget text_color.a
};
#endif // HELIORA_SCENE_STORY_H
