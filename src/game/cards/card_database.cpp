
#include "game/cards/card_database.h"
#include "utils/logger.h"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

// ── Lua helpers ──────────────────────────────────────────────────────────────

static std::string getStringField(lua_State *L, const char *key)
{
    lua_getfield(L, -1, key);
    std::string result;
    if (lua_isstring(L, -1))
        result = lua_tostring(L, -1);
    lua_pop(L, 1);
    return result;
}

static int32_t getIntField(lua_State *L, const char *key, int32_t fallback = 0)
{
    lua_getfield(L, -1, key);
    int32_t result = fallback;
    if (lua_isinteger(L, -1))
        result = static_cast<int32_t>(lua_tointeger(L, -1));
    else if (lua_isnumber(L, -1))
        result = static_cast<int32_t>(lua_tonumber(L, -1));
    lua_pop(L, 1);
    return result;
}

static bool getBoolField(lua_State *L, const char *key, bool fallback = false)
{
    lua_getfield(L, -1, key);
    bool result = fallback;
    if (lua_isboolean(L, -1))
        result = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    return result;
}

// ── load ─────────────────────────────────────────────────────────────────────

void CardDatabase::load(const std::string &path, std::shared_ptr<AssetsInterface> assets)
{
    m_cards.clear();

    std::string code = assets->loadText(path.c_str());
    if (code.empty()) {
        LOG_WARN("[CardDatabase] Script empty or not found");
        return;
    }

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_loadstring(L, code.c_str()) != LUA_OK) {
        LOG_WARN("[CardDatabase] Lua syntax error");
        lua_close(L);
        return;
    }

    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        LOG_WARN("[CardDatabase] Lua runtime error");
        lua_close(L);
        return;
    }

    if (!lua_istable(L, -1)) {
        LOG_WARN("[CardDatabase] Script must return a table");
        lua_close(L);
        return;
    }

    int len = static_cast<int>(lua_rawlen(L, -1));
    m_cards.reserve(len);

    for (int i = 1; i <= len; i++) {
        lua_rawgeti(L, -1, i);
        if (lua_istable(L, -1)) {
            CardInfo info;
            info.id = static_cast<int>(m_cards.size());
            info.lua_id = getStringField(L, "id");
            info.name = getStringField(L, "name");
            info.inner = getStringField(L, "inner");
            info.detail = getStringField(L, "description");
            info.type = getStringField(L, "type");
            info.cost = static_cast<uint8_t>(getIntField(L, "cost"));
            info.show = true;
            info.damage = getIntField(L, "damage");
            info.block = getIntField(L, "block");
            info.heal = getIntField(L, "heal");
            info.apply_vulnerable = static_cast<int8_t>(getIntField(L, "apply_vulnerable"));
            info.apply_weak = static_cast<int8_t>(getIntField(L, "apply_weak"));
            info.apply_strength = static_cast<int8_t>(getIntField(L, "apply_strength"));
            info.exhaust = getBoolField(L, "exhaust");
            {
                std::string a = getStringField(L, "art");
                if (!a.empty())
                    info.art = a;
            }
            m_cards.push_back(std::move(info));
        }
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
    lua_close(L);

    LOG_TRACE("[CardDatabase] Loaded cards");
}

// ── queries ──────────────────────────────────────────────────────────────────

const CardInfo *CardDatabase::findById(const std::string &lua_id) const
{
    for (const auto &c : m_cards)
        if (c.lua_id == lua_id)
            return &c;
    return nullptr;
}

const CardInfo *CardDatabase::findByIndex(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_cards.size()))
        return nullptr;
    return &m_cards[index];
}
