--------------------------------------------------------------------------------
-- ui_story.lua
-- State machine: FADE_IN → HOLD → FADE_OUT → next slide / scene change
-- Skip: click ระหว่าง FADE_IN/HOLD → เริ่ม FADE_OUT ทันที
--        click ระหว่าง FADE_OUT → ข้ามไป slide ถัดไปทันที
-- API ที่ใช้จริง:
--   UI.set_text_color(id, {r,g,b,a})  ← fade text
--   Input.mouse_clicked()              ← detect click
--   CallWithFloat → on_update(dt)      ← deltaTime จาก C++
--------------------------------------------------------------------------------

local SCREEN_W     = 1280
local SCREEN_H     = 720

local FADE_IN_SEC  = 1.2
local HOLD_SEC     = 2.8
local FADE_OUT_SEC = 1.0

-- ตำแหน่ง label กลางจอ
local TEXT_W       = 900
local TEXT_H       = 80
local TEXT_X       = (SCREEN_W - TEXT_W) / 2
local TEXT_Y       = SCREEN_H / 2 - 60
local FONT_SIZE    = 32

local HINT_H       = 36
local HINT_Y       = TEXT_Y + TEXT_H + 24
local HINT_FONT    = 17

-- ── Story lines ──────────────────────────────────────────────────────────────
-- เพิ่ม / แก้ key ใน en.json ได้เลย
local STORY_KEYS = {
    "story.line1",
    "story.line2",
    "story.line3",
    "story.line4",
    "story.line5",
    "story.line6",
    "story.line7",
    "story.line8",
    "story.line9",
    "story.line10",
    "story.line11",
    "story.line12",
    "story.line13",
    "story.line14",
    "story.line15",
    "story.line16",
    "story.line17",
    "story.line18",
    "story.line19",
    "story.line20",
    "story.line21",
    "story.line22",
    "story.line23",
    "story.line24",
    "story.line25",
    "story.line26",
    "story.line27",
}

local NEXT_SCENE = "menu"

-- ── State ────────────────────────────────────────────────────────────────────
local STATE_FADE_IN  = "FADE_IN"
local STATE_HOLD     = "HOLD"
local STATE_FADE_OUT = "FADE_OUT"
local STATE_DONE     = "DONE"

local cur_index   = 1
local cur_state   = STATE_FADE_IN
local timer       = 0.0
local skip_queued = false

-- ── Helpers ──────────────────────────────────────────────────────────────────

local function clamp01(t)
    return math.max(0.0, math.min(1.0, t))
end

-- ตั้งค่า alpha ของ story_text (text_color) และ story_hint
local function apply_alpha(text_a, hint_a)
    UI.set_text_color("story_text", {
        r = 255, g = 255, b = 255,
        a = math.floor(clamp01(text_a) * 255)
    })
    UI.set_text_color("story_hint", {
        r = 200, g = 200, b = 200,
        a = math.floor(clamp01(hint_a) * 255)
    })
end

local function load_slide(index)
    UI.set_text("story_text", tr(STORY_KEYS[index]))
    apply_alpha(0, 0)   -- เริ่มที่ transparent
end

-- ── Lifecycle ─────────────────────────────────────────────────────────────────

function on_enter()
    UI.clear()

    -- Label หลัก (เริ่ม invisible)
    UI.add_label("story_text", {
        x         = TEXT_X,
        y         = TEXT_Y,
        w         = TEXT_W,
        h         = TEXT_H,
        text      = "",
        font_size = FONT_SIZE,
        text_color = { r = 255, g = 255, b = 255, a = 0 },
        layer     = 1,
    })

    -- Hint "Click to continue"
    UI.add_label("story_hint", {
        x          = TEXT_X,
        y          = HINT_Y,
        w          = TEXT_W,
        h          = HINT_H,
        text       = tr("story.hint"),
        font_size  = HINT_FONT,
        text_color = { r = 200, g = 200, b = 200, a = 0 },
        layer      = 1,
    })

    UI.add_button("btn_skip", {
        x           = 1280 - 120 - 20,
        y           = 20,
        w           = 120,
        h           = 36,
        text        = tr("story.skip"),
        font_size   = 16,
        color       = { r = 255, g = 255, b = 255, a = 30 },
        hover_color = { r = 255, g = 255, b = 255, a = 60 },
        press_color = { r = 255, g = 255, b = 255, a = 15 },
        text_color  = { r = 255, g = 255, b = 255, a = 180 },
        layer       = 10,
        on_click    = function()
            cur_state = STATE_DONE
            Scene.change(NEXT_SCENE)
        end,
    })

    cur_index   = 1
    cur_state   = STATE_FADE_IN
    timer       = 0.0
    skip_queued = false

    load_slide(cur_index)
end

-- on_update(dt) เรียกจาก C++ ผ่าน CallWithFloat
function on_update(dt)
    -- รับ input
    if Input.mouse_clicked() then
        skip_queued = true
    end

    if cur_state == STATE_DONE then return end

    timer = timer + dt

    -- ── FADE IN ──────────────────────────────────────────────────────────────
    if cur_state == STATE_FADE_IN then
        local t = clamp01(timer / FADE_IN_SEC)
        apply_alpha(t, 0)

        if skip_queued then
            -- กด skip ระหว่าง fade in → ข้ามไป fade out ทันที
            skip_queued = false
            cur_state   = STATE_FADE_OUT
            timer       = 0.0
            apply_alpha(1, 0)
            return
        end

        if timer >= FADE_IN_SEC then
            apply_alpha(1, 0)
            cur_state = STATE_HOLD
            timer     = 0.0
        end

        -- ── HOLD ─────────────────────────────────────────────────────────────────
    elseif cur_state == STATE_HOLD then
        -- hint ค่อยๆ fade in ช่วง 0.4–1.0 วินาทีแรกของ HOLD
        local hint_t = clamp01((timer - 0.4) / 0.6)
        apply_alpha(1, hint_t * 0.6)   -- hint max alpha ~60%

        if skip_queued or timer >= HOLD_SEC then
            skip_queued = false
            cur_state   = STATE_FADE_OUT
            timer       = 0.0
        end

        -- ── FADE OUT ─────────────────────────────────────────────────────────────
    elseif cur_state == STATE_FADE_OUT then
        local t = clamp01(timer / FADE_OUT_SEC)
        apply_alpha(1 - t, (1 - t) * 0.6)

        if skip_queued then
            -- กด skip ระหว่าง fade out → จบทันที
            skip_queued = false
            timer       = FADE_OUT_SEC
        end

        if timer >= FADE_OUT_SEC then
            apply_alpha(0, 0)

            if cur_index < #STORY_KEYS then
                cur_index = cur_index + 1
                load_slide(cur_index)
                cur_state = STATE_FADE_IN
                timer     = 0.0
            else
                cur_state = STATE_DONE
                Scene.change(NEXT_SCENE)
            end
        end
    end
end

function on_exit()
    UI.clear()
end