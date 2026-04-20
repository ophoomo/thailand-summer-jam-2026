
#include "ui/ui_system.h"
#include "glm/ext/vector_float4.hpp"
#include "lauxlib.h"
#include "renderer/color.h"
#include "utils/logger.h"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

// ============================================================
// Construction / destruction
// ============================================================

UISystem::UISystem(lua_State *L, std::shared_ptr<OxRenderer> renderer)
{
    this->m_renderer = renderer;
    this->m_L = L;
}

// ============================================================
// Public Methods
// ============================================================

void UISystem::onUpdate(const MouseState &mouse)
{
    for (auto &w : m_widgets) {
        if (!w.visible || !w.enabled)
            continue;
        if (w.type != WidgetType::Button)
            continue;

        bool was_hovered = w.hovered;
        w.hovered = PointInRect(mouse.x, mouse.y, w.rect);

        // on_hover
        if (w.hovered && !was_hovered && w.lua_on_hover_ref != -1)
            CallLuaRef(w.lua_on_hover_ref);

        // on_unhover
        if (!w.hovered && was_hovered && w.lua_on_unhover_ref != -1)
            CallLuaRef(w.lua_on_unhover_ref);

        // on_click — fire on the frame the mouse button goes down inside
        if (w.hovered && mouse.clicked && w.lua_on_click_ref != -1) {
            w.pressed = true;
            CallLuaRef(w.lua_on_click_ref);
        }

        if (mouse.released)
            w.pressed = false;
    }
}

void UISystem::onDraw()
{
    for (const auto &w : m_widgets) {
        if (w.visible)
            this->DrawWidget(w);
    }
}

Widget *UISystem::AddPanel(const std::string &id, glm::vec4 rect, Color color, int32_t layer)
{
    this->m_widgets.emplace_back();
    auto &w = this->m_widgets.back();

    w.id = id;
    w.type = WidgetType::Panel;
    w.rect = rect;
    w.color = color;
    w.layer = layer;

    return &w;
}

Widget *UISystem::AddLabel(const std::string &id, glm::vec4 rect, const std::string &text,
                           float font_size, Color text_color, int32_t layer)
{
    this->m_widgets.emplace_back();
    auto &w = this->m_widgets.back();

    w.id = id;
    w.type = WidgetType::Label;
    w.rect = rect;
    w.text = text;
    w.font_size = font_size;
    w.text_color = text_color;
    w.color = Color::Transparent();
    w.layer = layer;

    return &w;
}

Widget *UISystem::AddButton(const std::string &id, glm::vec4 rect, const std::string &text,
                            float fontSize, int on_click_ref, Color color, int32_t layer)
{
    this->m_widgets.emplace_back();
    auto &w = this->m_widgets.back();

    w.id = id;
    w.type = WidgetType::Button;
    w.rect = rect;
    w.text = text;
    w.color = color;
    w.layer = layer;
    w.font_size = fontSize;

    auto clamp = [](int val) { return std::max(0, std::min(255, val)); };
    w.hover_color = {static_cast<unsigned char>(clamp(color.r + 38)),
                     static_cast<unsigned char>(clamp(color.g + 38)),
                     static_cast<unsigned char>(clamp(color.b + 38)), color.a};
    w.press_color = {static_cast<unsigned char>(clamp(color.r - 26)),
                     static_cast<unsigned char>(clamp(color.g - 26)),
                     static_cast<unsigned char>(clamp(color.b - 26)), color.a};
    w.lua_on_click_ref = on_click_ref;

    return &w;
}

Widget *UISystem::AddImage(const std::string &id, glm::vec4 rect, const std::string texture_name,
                           Color tint, int32_t layer)
{
    this->m_widgets.emplace_back();
    auto &w = this->m_widgets.back();

    w.id = id;
    w.type = WidgetType::Image;
    w.rect = rect;
    w.color = tint;
    w.texture_name = texture_name;
    w.layer = layer;

    return &w;
}

Widget *UISystem::Find(const std::string &id)
{
    for (auto &w : this->m_widgets)
        if (w.id == id)
            return &w;
    return nullptr;
}

const Widget *UISystem::Find(const std::string &id) const
{
    for (const auto &w : this->m_widgets)
        if (w.id == id)
            return &w;
    return nullptr;
}

void UISystem::SetVisible(const std::string &id, bool visible)
{
    if (auto *w = Find(id))
        w->visible = visible;
}

void UISystem::SetText(const std::string &id, const std::string &text)
{
    if (auto *w = Find(id))
        w->text = text;
}

void UISystem::SetEnabled(const std::string &id, bool enabled)
{
    if (auto *w = Find(id))
        w->enabled = enabled;
}

void UISystem::Remove(const std::string &id)
{
    auto it = std::find_if(this->m_widgets.begin(), this->m_widgets.end(),
                           [&id](const Widget &w) { return w.id == id; });
    if (it != this->m_widgets.end()) {
        this->ReleaseLuaRefs(*it);
        this->m_widgets.erase(it);
    }
}

void UISystem::Clear()
{
    for (auto &w : this->m_widgets)
        this->ReleaseLuaRefs(w);
    this->m_widgets.clear();
}

// ============================================================
// Private Methods
// ============================================================

void UISystem::DrawWidget(const Widget &w)
{
    switch (w.type) {
    case WidgetType::Label:
        this->DrawLabel(w);
        break;
    case WidgetType::Button:
        this->DrawButton(w);
        break;
    case WidgetType::Panel:
        this->DrawPanel(w);
        break;
    case WidgetType::Image:
        this->DrawImage(w);
        break;
    }
}

void UISystem::DrawImage(const Widget &w)
{
    this->m_renderer->oxDrawSprite(w.rect[0], w.rect[1], w.rect[2], w.rect[3], w.texture_name,
                                   w.color, w.layer);
}

void UISystem::DrawPanel(const Widget &w)
{
    this->m_renderer->oxDrawRectangle(w.rect[0], w.rect[1], w.rect[2], w.rect[3], w.color);
}

void UISystem::DrawLabel(const Widget &w)
{
    const float textWidth = this->m_renderer->measureText(w.text.c_str(), w.font_size);
    const auto &fm = this->m_renderer->fontMetrics();
    const float scale = w.font_size / fm.emSize;
    const float cx = w.rect[0] + (w.rect[2] - textWidth) * 0.5f;
    const float cy = w.rect[1] + w.rect[3] * 0.5f - (fm.ascender + fm.descender) * 0.5f * scale;
    this->m_renderer->oxDrawText(cx, cy, w.text.c_str(), w.font_size, w.text_color, w.text_effect,
                                 w.layer);
}

void UISystem::DrawButton(const Widget &w)
{
    Color fill;
    if (w.pressed)
        fill = w.press_color;
    else if (w.hovered)
        fill = w.hover_color;
    else
        fill = w.color;

    this->m_renderer->oxDrawRectangle(w.rect[0], w.rect[1], w.rect[2], w.rect[3], fill, w.layer);

    const float b = 2.f;
    Color border = w.text_color;

    // top
    this->m_renderer->oxDrawRectangle(w.rect[0], w.rect[1], w.rect[2], b, border, w.layer + 1);
    // bottom
    this->m_renderer->oxDrawRectangle(w.rect[0], w.rect[1] + w.rect[3] - b, w.rect[2], b, border,
                                      w.layer + 1);
    // left
    this->m_renderer->oxDrawRectangle(w.rect[0], w.rect[1], b, w.rect[3], border, w.layer + 1);
    // right
    this->m_renderer->oxDrawRectangle(w.rect[0] + w.rect[2] - b, w.rect[1], b, w.rect[3], border,
                                      w.layer + 1);

    const float textWidth = this->m_renderer->measureText(w.text.c_str(), w.font_size);
    const auto &fm = this->m_renderer->fontMetrics();
    const float scale = w.font_size / fm.emSize;

    // Horizontal: center measured text width inside button rect
    const float cx = w.rect[0] + (w.rect[2] - textWidth) * 0.5f;
    // Vertical: place baseline so visual midpoint of glyphs aligns with rect center
    // ascender/descender are in Y-down em space (ascender < 0 = above baseline)
    const float cy = w.rect[1] + w.rect[3] * 0.5f - (fm.ascender + fm.descender) * 0.5f * scale;
    this->m_renderer->oxDrawText(cx, cy, w.text.c_str(), w.font_size, w.text_color, w.text_effect,
                                 w.layer);
}

// ============================================================
// Lua callback helpers
// ============================================================

void UISystem::CallLuaRef(int ref)
{
    if (!this->m_L || ref == LUA_NOREF || ref == LUA_REFNIL)
        return;

    lua_rawgeti(this->m_L, LUA_REGISTRYINDEX, ref);
    if (lua_isfunction(this->m_L, -1)) {
        if (lua_pcall(this->m_L, 0, 0, 0) != LUA_OK) {
            LOG_CORE_ERROR("[UISystem] Lua callback error: {}", lua_tostring(this->m_L, -1));
            lua_pop(this->m_L, 1);
        }
    } else {
        lua_pop(this->m_L, 1);
    }
}

void UISystem::ReleaseLuaRefs(Widget &w)
{
    if (!this->m_L)
        return;
    auto release = [this](int &ref) {
        if (ref != -1 && ref != LUA_NOREF) {
            luaL_unref(this->m_L, LUA_REGISTRYINDEX, ref);
            ref = -1;
        }
    };
    release(w.lua_on_click_ref);
    release(w.lua_on_hover_ref);
    release(w.lua_on_unhover_ref);
}

bool UISystem::PointInRect(float px, float py, const glm::vec4 &r)
{
    return px >= r[0] && px <= r[0] + r[2] && py >= r[1] && py <= r[1] + r[3];
}
