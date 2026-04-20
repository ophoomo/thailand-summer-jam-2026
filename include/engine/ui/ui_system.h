#ifndef CFD6D1B0_A2E7_425C_A283_FED886DE0968
#define CFD6D1B0_A2E7_425C_A283_FED886DE0968

#include "glm/ext/vector_float4.hpp"
#include "renderer/color.h"
#include "renderer/ox_renderer.h"
#include "renderer/text_effect.h"
#include <cstdint>
#include <memory>
#include <string>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

// ============================================================
// Widget types
// ============================================================
enum class WidgetType { Panel, Label, Button, Image };

struct Widget
{
    std::string id;
    WidgetType type;
    Color color;
    glm::vec4 rect;
    bool visible = true;
    bool enabled = true;

    // Label / Button text
    std::string text = "";
    float font_size = 16.f;
    Color text_color;
    TextEffect text_effect;

    // Button visual states
    Color hover_color;
    Color press_color;
    bool hovered = false;
    bool pressed = false;

    // Lua registry reference to on_click function (-1 = none)
    // Set via luaL_ref(); released on widget removal.
    int lua_on_click_ref = -1;
    int lua_on_hover_ref = -1;
    int lua_on_unhover_ref = -1;

    std::string texture_name;

    int32_t layer = 0;
};

// ============================================================
// Mouse state (updated by SceneMenu each frame)
// ============================================================
struct MouseState
{
    float x = 0;
    float y = 0;
    bool clicked = false;  // true only on the frame the button goes down
    bool held = false;     // true while held
    bool released = false; // true on the frame the button goes up
};

class UISystem
{
  public:
    explicit UISystem(lua_State *L, std::shared_ptr<OxRenderer> renderer);
    ~UISystem() = default;

    void onUpdate(const MouseState &mouse);
    void onDraw();

    Widget *AddPanel(const std::string &id, glm::vec4 rect, Color color, int32_t layer = 0);
    Widget *AddLabel(const std::string &id, glm::vec4 rect, const std::string &text,
                     float font_size = 16.f, Color text_color = Color::White(), int32_t layer = 0);
    Widget *AddButton(const std::string &id, glm::vec4 rect, const std::string &text,
                      float fontSize = 16, int on_click_ref = -1, Color color = Color::White(),
                      int32_t layer = 0);
    Widget *AddImage(const std::string &id, glm::vec4 rect, const std::string texture_name,
                     Color tint = Color::White(), int32_t layer = 0);

    Widget *Find(const std::string &id);
    const Widget *Find(const std::string &id) const;
    void SetVisible(const std::string &id, bool visible);
    void SetText(const std::string &id, const std::string &text);
    void SetEnabled(const std::string &id, bool enabled);
    void Remove(const std::string &id);
    void Clear();

  private:
    void DrawWidget(const Widget &w);
    void DrawPanel(const Widget &w);
    void DrawLabel(const Widget &w);
    void DrawButton(const Widget &w);
    void DrawImage(const Widget &w);

    void CallLuaRef(int ref);
    void ReleaseLuaRefs(Widget &w);

    bool PointInRect(float px, float py, const glm::vec4 &r);

    lua_State *m_L;
    std::shared_ptr<OxRenderer> m_renderer;
    std::vector<Widget> m_widgets;
};

#endif /* CFD6D1B0_A2E7_425C_A283_FED886DE0968 */
