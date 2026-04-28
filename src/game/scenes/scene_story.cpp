#include "game/scenes/scene_story.h"
#include "core/localization.h"
#include "engine/core/scene_manager.h"
#include "engine/utils/logger.h"

// ─────────────────────────────────────────────────────────────────────────────
// Layout
// ─────────────────────────────────────────────────────────────────────────────
static constexpr float SW = 1280.0f;
static constexpr float SH = 720.0f;

// Story text block — centred horizontally, vertically around this Y
static constexpr float BLOCK_CENTER_Y = SH * 0.46f;
static constexpr float LINE_H = 46.0f;
static constexpr float TEXT_FONT = 28.0f;
static constexpr float TEXT_W = 900.0f;
static constexpr float TEXT_X = (SW - TEXT_W) * 0.5f;

// "Click to continue" hint
static constexpr float HINT_Y = BLOCK_CENTER_Y + 170.0f;
static constexpr float HINT_H = 30.0f;
static constexpr float HINT_FONT = 16.0f;

// Skip button (top-right)  — image bg + label on top
static constexpr float BTN_W = 120.0f;
static constexpr float BTN_H = 36.0f;
static constexpr float BTN_X = SW - BTN_W - 20.0f;
static constexpr float BTN_Y = 18.0f;

// Layers
static constexpr int32_t LAYER_BG = 0;
static constexpr int32_t LAYER_TEXT = 1;
static constexpr int32_t LAYER_BTN = 10;

// ─────────────────────────────────────────────────────────────────────────────
// Slide definitions  (mirror en.json groups)
// ─────────────────────────────────────────────────────────────────────────────
void SceneStory::buildSlides()
{
    // Pull strings through Localization so Thai works automatically.
    // Each inner list = one slide displayed together.
    m_slides = {
        {
            Localization::get("story.line1"),
            Localization::get("story.line2"),
            Localization::get("story.line3"),
            Localization::get("story.line4"),
        },
        {
            Localization::get("story.line5"),
            Localization::get("story.line6"),
            Localization::get("story.line7"),
            Localization::get("story.line8"),
        },
        {
            Localization::get("story.line9"),
            Localization::get("story.line10"),
            Localization::get("story.line11"),
            Localization::get("story.line12"),
            Localization::get("story.line13"),
        },
        {
            Localization::get("story.line14"),
            Localization::get("story.line15"),
            Localization::get("story.line16"),
            Localization::get("story.line17"),
        },
        {
            Localization::get("story.line18"),
            Localization::get("story.line19"),
            Localization::get("story.line20"),
            Localization::get("story.line21"),
            Localization::get("story.line22"),
        },
        {
            Localization::get("story.line23"),
            Localization::get("story.line24"),
            Localization::get("story.line25"),
            Localization::get("story.line26"),
            Localization::get("story.line27"),
        },
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// Build / rebuild story widgets for the current slide
// ─────────────────────────────────────────────────────────────────────────────
void SceneStory::buildStoryWidgets()
{
    // Remove previous per-slide labels (keep skip button & hint)
    for (int i = 0; i < 10; ++i)
        m_ui->Remove("line" + std::to_string(i));
    m_ui->Remove("hint");

    if (m_slideIndex >= static_cast<int>(m_slides.size()))
        return;

    const auto &lines = m_slides[m_slideIndex];
    const float blockH = static_cast<float>(lines.size()) * LINE_H;
    const float startY = BLOCK_CENTER_Y - blockH * 0.5f;

    for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
        float y = startY + static_cast<float>(i) * LINE_H;
        m_ui->AddLabel("line" + std::to_string(i), {TEXT_X, y, TEXT_W, LINE_H}, lines[i], TEXT_FONT,
                       Color{255, 255, 255, 0}, // start transparent
                       LAYER_TEXT);
    }

    // Hint
    m_ui->AddLabel("hint", {TEXT_X, HINT_Y, TEXT_W, HINT_H}, Localization::get("story.hint"),
                   HINT_FONT, Color{200, 200, 200, 0}, LAYER_TEXT);
}

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────
void SceneStory::onEnter()
{
    LOG_TRACE("[SceneStory] Enter");
    this->m_dispatcher->sink<WindowMouseEvent>().connect<&SceneStory::onMouse>(this);

    // UISystem needs a lua_State* — pass nullptr (no Lua in this scene)
    m_ui = std::make_unique<UISystem>(nullptr, m_renderer);

    // Load textures
    {
        int w, h, c;
        auto px = m_assets->loadImage("assets/images/splash_scene.png", w, h, c);
        m_renderer->createTexture("splash_scene", px, w, h);
        m_assets->unLoadImage(px);
    }
    {
        int w, h, c;
        auto px = m_assets->loadImage("assets/images/gui/button.png", w, h, c);
        m_renderer->createTexture("btn_skip", px, w, h);
        m_assets->unLoadImage(px);
    }

    {
        int channels, sample_rate;
        short *data;
        int sim =
            this->m_assets->loadAudio("assets/audio/gameplay_bgm.ogg", channels, sample_rate, data);
        this->m_audio->load("gameplay", channels, sim, sample_rate, data);
    }

    this->m_audio->set_bgm_fade_gain(0);
    this->m_audio->fade_bgm(1.0f, 5.0f);
    this->m_audio->play_bgm("gameplay", true, 0.1);

    // ── Static widgets (survive all slides) ──────────────────────────────────

    // Black background
    m_ui->AddPanel("bg", {0, 0, SW, SH}, Color{0, 0, 0, 255}, LAYER_BG);

    // Skip button: image bg + text label on top
    m_ui->AddImage("btn_skip_img", {BTN_X, BTN_Y, BTN_W, BTN_H}, "btn_skip", Color::White(),
                   LAYER_BTN);
    m_ui->AddLabel("btn_skip_txt", {BTN_X, BTN_Y, BTN_W, BTN_H}, Localization::get("story.skip"),
                   16.0f, Color{255, 255, 255, 200}, LAYER_BTN + 1);

    // ── Per-slide state ───────────────────────────────────────────────────────
    buildSlides();

    m_slideIndex = 0;
    m_state = State::FadeIn;
    m_timer = 0.0f;
    m_outroTimer = 0.0f;
    m_outroDone = false;
    m_mouseClick = false;
    m_mouseHeld = false;
    m_skipQueued = false;

    buildStoryWidgets();
}

void SceneStory::onExit()
{
    LOG_TRACE("[SceneStory] Exit");
    this->m_dispatcher->sink<WindowMouseEvent>().disconnect(this);
    m_ui.reset();
    m_renderer->freeTexture("splash_scene");
    m_renderer->freeTexture("btn_skip");
    this->m_audio->unload("gameplay");
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────
void SceneStory::onUpdate(double dt)
{
    // Consume click flag
    if (m_mouseClick) {
        m_skipQueued = true;
        m_mouseClick = false;
    }

    MouseState ms;
    ms.x = m_mouseX;
    ms.y = m_mouseY;
    ms.clicked = m_skipQueued; // drive UISystem hover/click
    m_ui->onUpdate(ms);

    if (m_state == State::Outro)
        updateOutro(static_cast<float>(dt));
    else if (m_state != State::Done)
        updateStory(static_cast<float>(dt));
}

// ─────────────────────────────────────────────────────────────────────────────
// State machine — story slides
// ─────────────────────────────────────────────────────────────────────────────
void SceneStory::updateStory(float dt)
{
    auto clamp01 = [](float v) -> float { return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v; };

    m_timer += dt;

    switch (m_state) {
    // ── FADE IN ──────────────────────────────────────────────────────────────
    case State::FadeIn: {
        applyAlpha(clamp01(m_timer / STORY_FADE_IN_SEC), 0.0f);

        if (m_skipQueued) {
            m_skipQueued = false;
            applyAlpha(1.0f, 0.0f);
            m_state = State::FadeOut;
            m_timer = 0.0f;
            break;
        }
        if (m_timer >= STORY_FADE_IN_SEC) {
            applyAlpha(1.0f, 0.0f);
            m_state = State::Hold;
            m_timer = 0.0f;
        }
        break;
    }

    // ── HOLD ─────────────────────────────────────────────────────────────────
    case State::Hold: {
        float hintT = clamp01((m_timer - 0.4f) / 0.6f) * 0.6f;
        applyAlpha(1.0f, hintT);

        if (m_skipQueued || m_timer >= STORY_HOLD_SEC) {
            m_skipQueued = false;
            m_state = State::FadeOut;
            m_timer = 0.0f;
        }
        break;
    }

    // ── FADE OUT ─────────────────────────────────────────────────────────────
    case State::FadeOut: {
        float t = clamp01(m_timer / STORY_FADE_OUT_SEC);
        applyAlpha(1.0f - t, (1.0f - t) * 0.6f);

        if (m_skipQueued) {
            m_skipQueued = false;
            m_timer = STORY_FADE_OUT_SEC; // force completion
        }
        if (m_timer >= STORY_FADE_OUT_SEC) {
            applyAlpha(0.0f, 0.0f);
            advanceSlide();
        }
        break;
    }

    default:
        break;
    }
}

void SceneStory::advanceSlide()
{
    m_slideIndex++;
    if (m_slideIndex < static_cast<int>(m_slides.size())) {
        buildStoryWidgets();
        m_state = State::FadeIn;
        m_timer = 0.0f;
    } else {
        // Hide story widgets, show outro image
        m_ui->SetVisible("hint", false);
        for (int i = 0; i < 10; ++i)
            m_ui->SetVisible("line" + std::to_string(i), false);
        m_ui->SetVisible("btn_skip_img", false);
        m_ui->SetVisible("btn_skip_txt", false);

        // Add outro image widget (starts transparent via tint)
        m_ui->AddImage("outro", {0, 0, SW, SH}, "splash_scene", Color{255, 255, 255, 0},
                       LAYER_BG + 1);

        m_state = State::Outro;
        m_outroTimer = 0.0f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State machine — outro splash image
// ─────────────────────────────────────────────────────────────────────────────
void SceneStory::updateOutro(float dt)
{
    m_outroTimer += dt;
    float t = m_outroTimer;

    float a = 0.0f;
    if (t < OUTRO_FADE_IN_SEC) {
        a = t / OUTRO_FADE_IN_SEC;
    } else if (t < OUTRO_FADE_IN_SEC + OUTRO_HOLD_SEC) {
        a = 1.0f;
    } else if (t < OUTRO_TOTAL_SEC) {
        a = 1.0f - (t - OUTRO_FADE_IN_SEC - OUTRO_HOLD_SEC) / OUTRO_FADE_OUT_SEC;
    } else {
        if (!m_outroDone) {
            m_outroDone = true;
            m_state = State::Done;
            m_dispatcher->trigger(SceneEvent{"menu"});
        }
        return;
    }

    // Drive tint alpha on the outro image widget
    if (auto *w = m_ui->Find("outro")) {
        w->color = Color{255, 255, 255, static_cast<uint8_t>(a * 255.0f)};
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Draw
// ─────────────────────────────────────────────────────────────────────────────
void SceneStory::onDraw()
{
    m_cursor->onDraw();
    m_ui->onDraw();
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
void SceneStory::applyAlpha(float textA, float hintA)
{
    auto toU8 = [](float v) -> uint8_t {
        return static_cast<uint8_t>((v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v) * 255.0f);
    };

    uint8_t ta = toU8(textA);
    uint8_t ha = toU8(hintA);

    for (int i = 0; i < 10; ++i) {
        if (auto *w = m_ui->Find("line" + std::to_string(i)))
            w->text_color.a = ta;
    }
    if (auto *w = m_ui->Find("hint"))
        w->text_color.a = ha;
}

// ─────────────────────────────────────────────────────────────────────────────
// Input
// ─────────────────────────────────────────────────────────────────────────────
void SceneStory::onMouse(const WindowMouseEvent &e)
{
    m_mouseX = static_cast<float>(e.mouseX);
    m_mouseY = static_cast<float>(e.mouseY);

    if (e.click) {
        if (!m_mouseHeld)
            m_mouseClick = true;
        m_mouseHeld = true;

        // Skip button hit-test — jump straight to outro
        if (m_state != State::Outro && m_state != State::Done) {
            bool inBtn = (m_mouseX >= BTN_X && m_mouseX <= BTN_X + BTN_W && m_mouseY >= BTN_Y &&
                          m_mouseY <= BTN_Y + BTN_H);
            if (inBtn) {
                m_slideIndex = static_cast<int>(m_slides.size());
                advanceSlide();
                m_mouseClick = false;
            }
        }
    } else {
        m_mouseHeld = false;
    }
}
