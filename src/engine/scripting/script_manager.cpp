
#include "scripting/script_manager.h"
#include "audio/audio_interface.h"
#include "core/application.h"
#include "core/localization.h"
#include "core/scene_manager.h"
#include "renderer/text_effect.h"
#include "utils/logger.h"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

// ============================================================
// Context passed as light userdata upvalue to all C functions.
// We keep one per LuaManager so all registered closures share
// the same pointers, making re-binding trivial.
// ============================================================
struct LuaCtx
{
    // App
    Application *app;
    // Scene
    entt::dispatcher *dispatcher = nullptr;
    std::string *current_scene_name = nullptr;
    // UI
    UISystem *ui = nullptr;
    // Input (updated by SceneMenu each frame before on_update call)
    float *mouse_x = nullptr;
    float *mouse_y = nullptr;
    bool *mouse_clicked = nullptr;
    bool *mouse_held = nullptr;
    // Audio
    AudioInterface *audio = nullptr;
};

// ============================================================
// Helpers: read values from a Lua table on the stack
// Static free functions — not class members.
// ============================================================

static int TblInt(lua_State *L, int idx, const char *key, int def = 0)
{
    lua_getfield(L, idx, key);
    int v = lua_isnumber(L, -1) ? (int)lua_tonumber(L, -1) : def;
    lua_pop(L, 1);
    return v;
}

static float TblFloat(lua_State *L, int idx, const char *key, float def = 0.f)
{
    lua_getfield(L, idx, key);
    float v = lua_isnumber(L, -1) ? float(lua_tonumber(L, -1)) : def;
    lua_pop(L, 1);
    return v;
}

static bool TblBool(lua_State *L, int idx, const char *key, bool def = false)
{
    lua_getfield(L, idx, key);
    bool v = lua_isboolean(L, -1) ? (bool)lua_toboolean(L, -1) : def;
    lua_pop(L, 1);
    return v;
}

static std::string TblString(lua_State *L, int idx, const char *key, const char *def = "")
{
    lua_getfield(L, idx, key);
    std::string v = lua_isstring(L, -1) ? lua_tostring(L, -1) : def;
    lua_pop(L, 1);
    return v;
}

// ============================================================
// Helper: retrieve LuaCtx stored as lightuserdata upvalue #1
// ============================================================
static LuaCtx *Ctx(lua_State *L)
{
    return static_cast<LuaCtx *>(lua_touserdata(L, lua_upvalueindex(1)));
}

// ============================================================
// Scene bindings
// ============================================================

static int lua_scene_change(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    if (auto *ctx = Ctx(L); ctx->dispatcher)
        ctx->dispatcher->trigger<SceneEvent>({std::string(name)});
    return 0;
}

static int lua_scene_current(lua_State *L)
{
    if (auto *ctx = Ctx(L); ctx->current_scene_name)
        lua_pushstring(L, ctx->current_scene_name->c_str());
    else
        lua_pushstring(L, "");
    return 1;
}

// ============================================================
// UI bindings
//
// Props table format (all fields optional with defaults):
//   x, y, w, h        — position/size in screen pixels
//   color = {r,g,b,a} — background colour
//   text              — label / button text
//   font_size         — text size (default 16)
//   text_color        — text colour
//   on_click          — Lua function (button only)
//   on_hover          — Lua function (button only)
//   on_unhover        — Lua function (button only)
//   tex_index         — texture slot index (image only)
// ============================================================

static glm::vec4 ReadRect(lua_State *L, int idx)
{
    return {TblFloat(L, idx, "x"), TblFloat(L, idx, "y"), TblFloat(L, idx, "w", 100),
            TblFloat(L, idx, "h", 40)};
}

static Color ReadColor(lua_State *L, int idx, const char *key, uint8_t dr = 255, uint8_t dg = 255,
                       uint8_t db = 255, uint8_t da = 255)
{
    lua_getfield(L, idx, key);

    if (lua_istable(L, -1)) {
        int ci = lua_gettop(L);

        dr = (uint8_t)TblInt(L, ci, "r", dr);
        dg = (uint8_t)TblInt(L, ci, "g", dg);
        db = (uint8_t)TblInt(L, ci, "b", db);
        da = (uint8_t)TblInt(L, ci, "a", da);
    }

    lua_pop(L, 1);
    return Color{dr, dg, db, da};
}

// Read an optional text_effect sub-table from a widget props table
static TextEffect ReadTextEffect(lua_State *L, int idx)
{
    lua_getfield(L, idx, "text_effect");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return TextEffect::None();
    }
    int ei = lua_gettop(L);
    std::string type = TblString(L, ei, "type", "none");
    TextEffect effect = TextEffect::None();
    if (type == "outline") {
        auto c = ReadColor(L, ei, "color", 0, 0, 0, 255);
        float width = TblFloat(L, ei, "width", 0.15f);
        effect = TextEffect::Outline(c, width);
    } else if (type == "glow") {
        auto c = ReadColor(L, ei, "color", 255, 255, 0, 255);
        float range = TblFloat(L, ei, "range", 0.3f);
        float strength = TblFloat(L, ei, "strength", 1.0f);
        effect = TextEffect::Glow(c, range, strength);
    }
    lua_pop(L, 1);
    return effect;
}

// Read an optional Lua function at table[key], push into registry, return ref
static int ReadCallbackRef(lua_State *L, int tbl_idx, const char *key)
{
    lua_getfield(L, tbl_idx, key);
    if (lua_isfunction(L, -1))
        return luaL_ref(L, LUA_REGISTRYINDEX); // pops the function
    lua_pop(L, 1);
    return -1;
}

static int lua_ui_add_panel(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    auto *ui = Ctx(L)->ui;
    if (!ui)
        return 0;

    auto rect = ReadRect(L, 2);
    auto color = ReadColor(L, 2, "color", 255, 255, 255, 255);
    auto layer = TblInt(L, 2, "layer", 0);
    ui->AddPanel(id, rect, color, layer);
    return 0;
}

static int lua_ui_add_label(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    auto *ui = Ctx(L)->ui;
    if (!ui)
        return 0;

    auto rect = ReadRect(L, 2);
    auto text = TblString(L, 2, "text");
    float font_size = TblFloat(L, 2, "font_size", 16.f);
    auto text_color = ReadColor(L, 2, "text_color", 255, 255, 255, 255);
    auto bg_color = ReadColor(L, 2, "color", 0, 0, 0, 0);
    auto effect = ReadTextEffect(L, 2);

    auto *w = ui->AddLabel(id, rect, text, font_size, text_color);
    if (w) {
        w->color = bg_color;
        w->text_effect = effect;
    }
    return 0;
}

static int lua_ui_add_button(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    auto *ui = Ctx(L)->ui;
    if (!ui)
        return 0;

    auto rect = ReadRect(L, 2);
    auto text = TblString(L, 2, "text");
    auto fontSize = TblFloat(L, 2, "font_size", 16.0);
    auto color = ReadColor(L, 2, "color", 0, 0, 0, 255);
    int on_click = ReadCallbackRef(L, 2, "on_click");
    int on_hover = ReadCallbackRef(L, 2, "on_hover");
    int on_unhover = ReadCallbackRef(L, 2, "on_unhover");
    int layer = TblInt(L, 2, "layer", 0);
    auto effect = ReadTextEffect(L, 2);

    auto *w = ui->AddButton(id, rect, text, fontSize, on_click, color, layer);
    if (w) {
        w->lua_on_hover_ref = on_hover;
        w->lua_on_unhover_ref = on_unhover;
        w->text_effect = effect;
        // Override hover/press colours if provided
        auto hc = ReadColor(L, 2, "hover_color", 0, 0, 0, 255);
        if (hc.r >= 0)
            w->hover_color = hc;
        auto pc = ReadColor(L, 2, "press_color", 0, 0, 0, 255);
        if (pc.r >= 0)
            w->press_color = pc;
    }
    return 0;
}

static int lua_ui_add_image(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    auto *ui = Ctx(L)->ui;
    if (!ui)
        return 0;

    auto rect = ReadRect(L, 2);
    auto texture_name = TblString(L, 2, "texture_name", 0);
    auto tint = ReadColor(L, 2, "color", 255, 255, 255, 255);
    int layer = TblInt(L, 2, "layer", 0);

    ui->AddImage(id, rect, texture_name, tint, layer);
    return 0;
}

static int lua_ui_clear(lua_State *L)
{
    if (auto *ui = Ctx(L)->ui)
        ui->Clear();
    return 0;
}

static int lua_ui_set_visible(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    bool v = lua_toboolean(L, 2);
    if (auto *ui = Ctx(L)->ui)
        ui->SetVisible(id, v);
    return 0;
}

static int lua_ui_set_text(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    const char *text = luaL_checkstring(L, 2);
    if (auto *ui = Ctx(L)->ui)
        ui->SetText(id, text);
    return 0;
}

static int lua_ui_set_enabled(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    bool v = lua_toboolean(L, 2);
    if (auto *ui = Ctx(L)->ui)
        ui->SetEnabled(id, v);
    return 0;
}

static int lua_ui_set_text_effect(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    auto *ui = Ctx(L)->ui;
    if (!ui)
        return 0;
    auto *w = ui->Find(id);
    if (!w)
        return 0;
    std::string type = TblString(L, 2, "type", "none");
    if (type == "outline") {
        auto c = ReadColor(L, 2, "color", 0, 0, 0, 255);
        float width = TblFloat(L, 2, "width", 0.15f);
        w->text_effect = TextEffect::Outline(c, width);
    } else if (type == "glow") {
        auto c = ReadColor(L, 2, "color", 255, 255, 0, 255);
        float range = TblFloat(L, 2, "range", 0.3f);
        float strength = TblFloat(L, 2, "strength", 1.0f);
        w->text_effect = TextEffect::Glow(c, range, strength);
    } else {
        w->text_effect = TextEffect::None();
    }
    return 0;
}

// ============================================================
// Input bindings
// ============================================================

static int lua_input_mouse_x(lua_State *L)
{
    auto *ctx = Ctx(L);
    lua_pushnumber(L, ctx->mouse_x ? *ctx->mouse_x : 0.f);
    return 1;
}

static int lua_input_mouse_y(lua_State *L)
{
    auto *ctx = Ctx(L);
    lua_pushnumber(L, ctx->mouse_y ? *ctx->mouse_y : 0.f);
    return 1;
}

static int lua_input_mouse_down(lua_State *L)
{
    auto *ctx = Ctx(L);
    lua_pushboolean(L, ctx->mouse_held && *ctx->mouse_held);
    return 1;
}

static int lua_input_mouse_clicked(lua_State *L)
{
    auto *ctx = Ctx(L);
    lua_pushboolean(L, ctx->mouse_clicked && *ctx->mouse_clicked);
    return 1;
}

// ============================================================
// App bindings
// ============================================================

static int lua_app_quit(lua_State *L)
{
    if (auto *dis = Ctx(L)->dispatcher) {
        dis->trigger<ApplicationEvent>({.shuntdown = true});
    }
    return 0;
}

// ============================================================
// Audio bindings
// ============================================================

static int lua_audio_play_sfx(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    float vol = lua_isnumber(L, 2) ? (float)lua_tonumber(L, 2) : 1.f;
    if (auto *a = Ctx(L)->audio)
        a->play_sfx(name, vol);
    return 0;
}

static int lua_audio_stop_all_sfx(lua_State *L)
{
    if (auto *a = Ctx(L)->audio)
        a->stop_all_sfx();
    return 0;
}

static int lua_audio_play_sfx_3d(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    float vol = lua_isnumber(L, 5) ? (float)lua_tonumber(L, 5) : 1.f;
    if (auto *a = Ctx(L)->audio)
        a->play_sfx_3d(name, x, y, z, vol);
    return 0;
}

static int lua_audio_play_bgm(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    bool loop = lua_isboolean(L, 2) ? (bool)lua_toboolean(L, 2) : true;
    float vol = lua_isnumber(L, 3) ? (float)lua_tonumber(L, 3) : 1.f;
    if (auto *a = Ctx(L)->audio)
        a->play_bgm(name, loop, vol);
    return 0;
}

static int lua_audio_stop_bgm(lua_State *L)
{
    if (auto *a = Ctx(L)->audio)
        a->stop_bgm();
    return 0;
}

static int lua_audio_pause_bgm(lua_State *L)
{
    if (auto *a = Ctx(L)->audio)
        a->pause_bgm();
    return 0;
}

static int lua_audio_resume_bgm(lua_State *L)
{
    if (auto *a = Ctx(L)->audio)
        a->resume_bgm();
    return 0;
}

static int lua_audio_set_master_volume(lua_State *L)
{
    if (auto *a = Ctx(L)->audio)
        a->set_master_volume((float)luaL_checknumber(L, 1));
    return 0;
}

static int lua_audio_set_sfx_volume(lua_State *L)
{
    if (auto *a = Ctx(L)->audio)
        a->set_sfx_volume((float)luaL_checknumber(L, 1));
    return 0;
}

static int lua_audio_set_bgm_volume(lua_State *L)
{
    if (auto *a = Ctx(L)->audio)
        a->set_bgm_volume((float)luaL_checknumber(L, 1));
    return 0;
}

static int lua_audio_get_master_volume(lua_State *L)
{
    lua_pushnumber(L, Ctx(L)->audio ? Ctx(L)->audio->get_master_volume() : 0.f);
    return 1;
}

static int lua_audio_get_sfx_volume(lua_State *L)
{
    lua_pushnumber(L, Ctx(L)->audio ? Ctx(L)->audio->get_sfx_volume() : 0.f);
    return 1;
}

static int lua_audio_get_bgm_volume(lua_State *L)
{
    lua_pushnumber(L, Ctx(L)->audio ? Ctx(L)->audio->get_bgm_volume() : 0.f);
    return 1;
}

static int lua_audio_get_bgm_state(lua_State *L)
{
    lua_pushstring(L, Ctx(L)->audio ? Ctx(L)->audio->get_bgm_state().c_str() : "stopped");
    return 1;
}

static int lua_audio_get_bgm_name(lua_State *L)
{
    lua_pushstring(L, Ctx(L)->audio ? Ctx(L)->audio->get_bgm_name().c_str() : "");
    return 1;
}

static int lua_audio_get_bgm_position(lua_State *L)
{
    lua_pushnumber(L, Ctx(L)->audio ? Ctx(L)->audio->get_bgm_position() : 0.f);
    return 1;
}

static int lua_audio_get_bgm_duration(lua_State *L)
{
    lua_pushnumber(L, Ctx(L)->audio ? Ctx(L)->audio->get_bgm_duration() : 0.f);
    return 1;
}

// ============================================================
// Localization bindings — no ctx needed (Localization is static)
// ============================================================

// Lang.tr("menu.start")  →  localised string
static int lua_lang_tr(lua_State *L)
{
    const char *key = luaL_checkstring(L, 1);
    lua_pushstring(L, Localization::get(key).c_str());
    return 1;
}

// Lang.set("th" | "en")
static int lua_lang_set(lua_State *L)
{
    const char *lang = luaL_checkstring(L, 1);
    if (std::string_view(lang) == "th")
        Localization::setLanguage(Language::Thai);
    else
        Localization::setLanguage(Language::English);
    return 0;
}

// Lang.get()  →  "th" | "en"
static int lua_lang_get(lua_State *L)
{
    lua_pushstring(L, Localization::isThai() ? "th" : "en");
    return 1;
}

// Lang.toggle()  — flip between Thai ↔ English
static int lua_lang_toggle(lua_State *L)
{
    Localization::setLanguage(Localization::isThai() ? Language::English : Language::Thai);
    lua_pushstring(L, Localization::isThai() ? "th" : "en");
    return 1;
}

static int lua_ui_set_color(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    auto *ui = Ctx(L)->ui;
    if (!ui) return 0;
    auto *w = ui->Find(id);
    if (!w) return 0;
    int idx = 2;
    w->color.r = (uint8_t)TblInt(L, idx, "r", w->color.r);
    w->color.g = (uint8_t)TblInt(L, idx, "g", w->color.g);
    w->color.b = (uint8_t)TblInt(L, idx, "b", w->color.b);
    w->color.a = (uint8_t)TblInt(L, idx, "a", w->color.a);
    return 0;
}

static int lua_ui_set_text_color(lua_State *L)
{
    const char *id = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    auto *ui = Ctx(L)->ui;
    if (!ui) return 0;
    auto *w = ui->Find(id);
    if (!w) return 0;
    int idx = 2;
    w->text_color.r = (uint8_t)TblInt(L, idx, "r", w->text_color.r);
    w->text_color.g = (uint8_t)TblInt(L, idx, "g", w->text_color.g);
    w->text_color.b = (uint8_t)TblInt(L, idx, "b", w->text_color.b);
    w->text_color.a = (uint8_t)TblInt(L, idx, "a", w->text_color.a);
    return 0;
}

// ============================================================
// Helper: register a table of {name → C function} as a Lua global,
// all sharing the same LuaCtx upvalue.
// ============================================================

static void RegisterTable(lua_State *L, LuaCtx *ctx, const char *global_name, const luaL_Reg *fns)
{
    lua_newtable(L);
    for (const luaL_Reg *f = fns; f->name; ++f) {
        lua_pushlightuserdata(L, ctx);
        lua_pushcclosure(L, f->func, 1);
        lua_setfield(L, -2, f->name);
    }
    lua_setglobal(L, global_name);
}

// ============================================================
// Construction / destruction
// ============================================================

ScriptManager::ScriptManager()
{
    this->m_L = luaL_newstate();
    luaL_openlibs(this->m_L);

    // Store the context struct in the Lua registry so it outlives calls.
    // We allocate it as a full userdata (managed by Lua GC).
    LuaCtx *ctx = static_cast<LuaCtx *>(lua_newuserdatauv(this->m_L, sizeof(LuaCtx), 0));
    new (ctx) LuaCtx{}; // placement new (zero-init)
    lua_setfield(this->m_L, LUA_REGISTRYINDEX, "__ox_ctx");
}

ScriptManager::~ScriptManager()
{
    if (this->m_L) {
        lua_close(this->m_L);
        this->m_L = nullptr;
    }
}

// ============================================================
// Get (or re-get) the context from the registry
// ============================================================

static LuaCtx *GetCtx(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "__ox_ctx");
    auto *ctx = static_cast<LuaCtx *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return ctx;
}

// ============================================================
// Binding registration
// ============================================================

void ScriptManager::BindScene(entt::dispatcher *dispatcher, std::string current_scene_name)
{
    LuaCtx *ctx = GetCtx(m_L);
    ctx->dispatcher = dispatcher;
    ctx->current_scene_name = &current_scene_name;

    static const luaL_Reg fns[] = {
        {"change", lua_scene_change}, {"current", lua_scene_current}, {nullptr, nullptr}};
    RegisterTable(m_L, ctx, "Scene", fns);
}

void ScriptManager::BindUI(UISystem *ui)
{
    LuaCtx *ctx = GetCtx(m_L);
    ctx->ui = ui;

    static const luaL_Reg fns[] = {{"add_panel", lua_ui_add_panel},
                                   {"add_label", lua_ui_add_label},
                                   {"add_button", lua_ui_add_button},
                                   {"add_image", lua_ui_add_image},
                                   {"clear", lua_ui_clear},
                                   {"set_visible", lua_ui_set_visible},
                                   {"set_text", lua_ui_set_text},
                                   {"set_enabled", lua_ui_set_enabled},
                                   {"set_text_effect", lua_ui_set_text_effect},
                                   {"set_color",       lua_ui_set_color},
                                   {"set_text_color",  lua_ui_set_text_color},
                                   {nullptr, nullptr}};
    RegisterTable(m_L, ctx, "UI", fns);
}

void ScriptManager::BindInput(float *mx, float *my, bool *clicked, bool *held)
{
    LuaCtx *ctx = GetCtx(m_L);
    ctx->mouse_x = mx;
    ctx->mouse_y = my;
    ctx->mouse_clicked = clicked;
    ctx->mouse_held = held;

    static const luaL_Reg fns[] = {{"mouse_x", lua_input_mouse_x},
                                   {"mouse_y", lua_input_mouse_y},
                                   {"mouse_down", lua_input_mouse_down},
                                   {"mouse_clicked", lua_input_mouse_clicked},
                                   {nullptr, nullptr}};
    RegisterTable(m_L, ctx, "Input", fns);
}

void ScriptManager::BindApp(entt::dispatcher *dispatcher)
{
    LuaCtx *ctx = GetCtx(m_L);
    ctx->dispatcher = dispatcher;
    static const luaL_Reg fns[] = {{"quit", lua_app_quit}, {nullptr, nullptr}};
    RegisterTable(m_L, ctx, "App", fns);
}

void ScriptManager::BindAudio(AudioInterface *audio)
{
    LuaCtx *ctx = GetCtx(m_L);
    ctx->audio = audio;

    static const luaL_Reg fns[] = {{"play_sfx", lua_audio_play_sfx},
                                   {"stop_all_sfx", lua_audio_stop_all_sfx},
                                   {"play_sfx_3d", lua_audio_play_sfx_3d},
                                   {"play_bgm", lua_audio_play_bgm},
                                   {"stop_bgm", lua_audio_stop_bgm},
                                   {"pause_bgm", lua_audio_pause_bgm},
                                   {"resume_bgm", lua_audio_resume_bgm},
                                   {"set_master_volume", lua_audio_set_master_volume},
                                   {"set_sfx_volume", lua_audio_set_sfx_volume},
                                   {"set_bgm_volume", lua_audio_set_bgm_volume},
                                   {"get_master_volume", lua_audio_get_master_volume},
                                   {"get_sfx_volume", lua_audio_get_sfx_volume},
                                   {"get_bgm_volume", lua_audio_get_bgm_volume},
                                   {"get_bgm_state", lua_audio_get_bgm_state},
                                   {"get_bgm_name", lua_audio_get_bgm_name},
                                   {"get_bgm_position", lua_audio_get_bgm_position},
                                   {"get_bgm_duration", lua_audio_get_bgm_duration},
                                   {nullptr, nullptr}};
    RegisterTable(m_L, ctx, "Audio", fns);
}

void ScriptManager::BindLocalization()
{
    // Lang table: Lang.tr / Lang.set / Lang.get / Lang.toggle
    lua_newtable(m_L);
    lua_pushcfunction(m_L, lua_lang_tr);
    lua_setfield(m_L, -2, "tr");
    lua_pushcfunction(m_L, lua_lang_set);
    lua_setfield(m_L, -2, "set");
    lua_pushcfunction(m_L, lua_lang_get);
    lua_setfield(m_L, -2, "get");
    lua_pushcfunction(m_L, lua_lang_toggle);
    lua_setfield(m_L, -2, "toggle");
    lua_setglobal(m_L, "Lang");

    // Convenience global: tr("key") === Lang.tr("key")
    lua_pushcfunction(m_L, lua_lang_tr);
    lua_setglobal(m_L, "tr");
}

// ============================================================
// Script execution
// ============================================================

bool ScriptManager::RunScript(const std::string name, const std::string &code)
{
    if (luaL_loadbuffer(this->m_L, code.data(), code.size(), name.c_str()) != LUA_OK) {
        LOG_CORE_ERROR("[ScriptManager] Error: {}", lua_tostring(this->m_L, -1));
        lua_pop(this->m_L, 1);
        return false;
    }
    if (lua_pcall(this->m_L, 0, LUA_MULTRET, 0) != LUA_OK) {
        LOG_CORE_ERROR("[ScriptManager] Runtime Error: {}", lua_tostring(this->m_L, -1));
        lua_pop(this->m_L, 1);
        return false;
    }
    return true;
}

bool ScriptManager::CallVoid(const std::string &fn)
{
    lua_getglobal(this->m_L, fn.c_str());
    if (!lua_isfunction(this->m_L, -1)) {
        lua_pop(this->m_L, 1);
        return false; // function doesn't exist — not an error
    }
    if (lua_pcall(this->m_L, 0, 0, 0) != LUA_OK) {
        LOG_CORE_ERROR("[ScriptManager] Error in '{}': {}", fn, lua_tostring(this->m_L, -1));
        lua_pop(this->m_L, 1);
        return false;
    }
    return true;
}

bool ScriptManager::CallWithFloat(const std::string &fn, float arg)
{
    lua_getglobal(this->m_L, fn.c_str());
    if (!lua_isfunction(this->m_L, -1)) {
        lua_pop(this->m_L, 1);
        return false;
    }
    lua_pushnumber(this->m_L, double(arg));
    if (lua_pcall(this->m_L, 1, 0, 0) != LUA_OK) {
        LOG_CORE_ERROR("[ScriptManager] Error in '{}': {}", fn, lua_tostring(this->m_L, -1));
        lua_pop(this->m_L, 1);
        return false;
    }
    return true;
}
