local SCREEN_W        = 1280
local SCREEN_H        = 720

-- Logo --
local LOGO_W = 330
local LOGO_H = 300
local LOGO_X = 80
local LOGO_Y = 80

-- Button layout constants (easy to tweak)
local BTN_X           = 140
local BTN_W           = 220
local BTN_H           = 52
local BTN_GAP         = 16
local BTN_START       = 400 -- Y of first button

local BTN_TEXT_COLOR  = { r = 255, g = 255, b = 255, a = 255 }

local onHoverAudio    = function() Audio.play_sfx("click", 1.0) end

local function btn_y(index)
    return BTN_START + index * (BTN_H + BTN_GAP)
end

function on_enter()
    UI.clear()

    UI.add_image("logo", {
        x            = LOGO_X,
        y            = LOGO_Y,
        w            = LOGO_W,
        h            = LOGO_H,
        texture_name = "heliola_logo",
        color        = { r = 255, g = 255, b = 255, a = 255 },
        layer        = 1,
    })

    UI.add_button("btn_play", {
        x           = BTN_X,
        y           = btn_y(0),
        w           = BTN_W,
        h           = BTN_H,
        text        = tr("menu.start"),
        color       = { r = 255, g = 255, b = 255, a = 0 },
        hover_color = { r = 255, g = 255, b = 255, a = 10 },
        press_color = { r = 255, g = 255, b = 255, a = 40 },
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

    -- Quit
    UI.add_button("btn_quit", {
        x           = BTN_X,
        y           = btn_y(1),
        w           = BTN_W,
        h           = BTN_H,
        text        = tr("menu.quit"),
        layer       = 1,

        color       = { r = 255, g = 255, b = 255, a = 0 },
        hover_color = { r = 255, g = 255, b = 255, a = 10 },
        press_color = { r = 255, g = 255, b = 255, a = 40 },

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
