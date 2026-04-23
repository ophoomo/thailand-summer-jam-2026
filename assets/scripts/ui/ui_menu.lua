local SCREEN_W        = 1280
local SCREEN_H        = 720

-- Button layout constants (easy to tweak)
local BTN_X           = (SCREEN_W - 220) / 2 -- horizontally centred
local BTN_W           = 220
local BTN_H           = 52
local BTN_GAP         = 16
local BTN_START       = 300 -- Y of first button

local BTN_COLOR       = { r = 38, g = 38, b = 56, a = 235 }
local BTN_HOVER_COLOR = { r = 77, g = 77, b = 115, a = 255 }
local BTN_PRESS_COLOR = { r = 20, g = 20, b = 31, a = 255 }
local BTN_TEXT_COLOR  = { r = 255, g = 255, b = 255, a = 255 }

local onHoverAudio    = function() Audio.play_sfx("click", 1.0) end

local function btn_y(index)
    return BTN_START + index * (BTN_H + BTN_GAP)
end

function on_enter()
    UI.clear()

    UI.add_button("btn_play", {
        x           = BTN_X,
        y           = btn_y(0),
        w           = BTN_W,
        h           = BTN_H,
        text        = tr("menu.start"),
        color       = BTN_COLOR,
        hover_color = BTN_HOVER_COLOR,
        press_color = BTN_PRESS_COLOR,
        text_color  = BTN_TEXT_COLOR,
        layer       = 1,

        on_click    = function()
            Scene.change("gameplay")
        end,

        on_hover    = function()
            UI.set_text("hint", "Start a new game")
            onHoverAudio()
        end,

        on_unhover  = function()
            UI.set_text("hint", "")
        end
    })

    -- Settings
    --UI.add_button("btn_setting", {
    --    x           = BTN_X,
    --    y           = btn_y(1),
    --    w           = BTN_W,
    --    h           = BTN_H,
    --    text        = tr("menu.settings"),
    --    color       = BTN_COLOR,
    --    hover_color = BTN_HOVER_COLOR,
    --    press_color = BTN_PRESS_COLOR,
    --    text_color  = BTN_TEXT_COLOR,
    --    layer       = 1,
    --
    --    on_click    = function()
    --        Scene.change("setting")
    --    end,
    --
    --    on_hover    = function()
    --        UI.set_text("hint", "Adjust audio, graphics and controls")
    --        onHoverAudio()
    --    end,
    --
    --    on_unhover  = function()
    --        UI.set_text("hint", "")
    --    end
    --})

    -- Credits
    --UI.add_button("btn_credit", {
    --    x           = BTN_X,
    --    y           = btn_y(1),
    --    w           = BTN_W,
    --    h           = BTN_H,
    --    text        = tr("menu.credits"),
    --    color       = BTN_COLOR,
    --    hover_color = BTN_HOVER_COLOR,
    --    press_color = BTN_PRESS_COLOR,
    --    text_color  = BTN_TEXT_COLOR,
    --    layer       = 1,
    --
    --    on_click    = function()
    --        Scene.change("credit")
    --    end,
    --
    --    on_hover    = function()
    --        UI.set_text("hint", "View the team")
    --        onHoverAudio()
    --    end,
    --
    --    on_unhover  = function()
    --        UI.set_text("hint", "")
    --    end
    --})

    -- Quit
    UI.add_button("btn_quit", {
        x           = BTN_X,
        y           = btn_y(1),
        w           = BTN_W,
        h           = BTN_H,
        text        = tr("menu.quit"),
        layer       = 1,

        color       = { r = 51, g = 20, b = 20, a = 235 },
        hover_color = { r = 102, g = 31, b = 31, a = 255 },
        press_color = { r = 26, g = 10, b = 10, a = 255 },

        text_color  = BTN_TEXT_COLOR,

        on_click    = function()
            App.quit()
        end,

        on_hover    = function()
            UI.set_text("hint", "Exit the game")
            onHoverAudio()
        end,

        on_unhover  = function()
            UI.set_text("hint", "")
        end
    })
end
