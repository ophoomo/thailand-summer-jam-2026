
#include "game/battle/enemy_database.h"
#include "utils/logger.h"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

namespace battle {

// ─────────────────────────────────────────────────────────────────────────────
//  Lua field helpers  (same pattern as CardDatabase)
// ─────────────────────────────────────────────────────────────────────────────

static std::string luaGetString(lua_State* L, const char* key)
{
    lua_getfield(L, -1, key);
    std::string result;
    if (lua_isstring(L, -1))
        result = lua_tostring(L, -1);
    lua_pop(L, 1);
    return result;
}

static int32_t luaGetInt(lua_State* L, const char* key, int32_t fallback = 0)
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

static bool luaGetBool(lua_State* L, const char* key, bool fallback = false)
{
    lua_getfield(L, -1, key);
    bool result = fallback;
    if (lua_isboolean(L, -1))
        result = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  String → enum converters  (done once at load time, zero runtime cost later)
// ─────────────────────────────────────────────────────────────────────────────

static EnemyType parseEnemyType(const std::string& s)
{
    if (s == "goblin")      return EnemyType::GOBLIN;
    if (s == "troll")       return EnemyType::TROLL;
    if (s == "archer")      return EnemyType::ARCHER;
    if (s == "dark_knight") return EnemyType::DARK_KNIGHT;
    if (s == "necromancer") return EnemyType::NECROMANCER;
    if (s == "boss_dragon") return EnemyType::BOSS_DRAGON;
    if (s == "boss_lich")   return EnemyType::BOSS_LICH;
    return EnemyType::SKELETON; // default
}

static IntentType parseIntentType(const std::string& s)
{
    if (s == "defend")  return IntentType::DEFEND;
    if (s == "buff")    return IntentType::BUFF;
    if (s == "debuff")  return IntentType::DEBUFF;
    if (s == "special") return IntentType::SPECIAL;
    return IntentType::ATTACK;  // default
}

// ─────────────────────────────────────────────────────────────────────────────
//  parsePatterns — reads the "patterns" array from the table on top of the stack
// ─────────────────────────────────────────────────────────────────────────────

static std::vector<AIPatternInfo> parsePatterns(lua_State* L)
{
    std::vector<AIPatternInfo> result;

    lua_getfield(L, -1, "patterns");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        LOG_WARN("[EnemyDatabase] Enemy has no 'patterns' table");
        return result;
    }

    int len = static_cast<int>(lua_rawlen(L, -1));
    result.reserve(static_cast<size_t>(len));

    for (int i = 1; i <= len; ++i) {
        lua_rawgeti(L, -1, i);
        if (lua_istable(L, -1)) {
            AIPatternInfo p;
            p.action           = parseIntentType(luaGetString(L, "action"));
            p.damage           = luaGetInt(L, "damage");
            p.block            = luaGetInt(L, "block");
            p.times            = luaGetInt(L, "times", 1);
            p.apply_vulnerable = static_cast<int8_t>(luaGetInt(L, "apply_vulnerable"));
            p.apply_weak       = static_cast<int8_t>(luaGetInt(L, "apply_weak"));
            result.push_back(p);
        }
        lua_pop(L, 1);
    }

    lua_pop(L, 1); // pop "patterns" table
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  load
// ─────────────────────────────────────────────────────────────────────────────

void EnemyDatabase::load(const std::string& path,
                          std::shared_ptr<AssetsInterface> assets)
{
    m_enemies.clear();

    std::string code = assets->loadText(path.c_str());
    if (code.empty()) {
        LOG_WARN("[EnemyDatabase] Script empty or not found: {}", path);
        return;
    }

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_loadstring(L, code.c_str()) != LUA_OK) {
        LOG_WARN("[EnemyDatabase] Lua syntax error in {}: {}",
                 path, lua_tostring(L, -1));
        lua_close(L);
        return;
    }

    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        LOG_WARN("[EnemyDatabase] Lua runtime error in {}: {}",
                 path, lua_tostring(L, -1));
        lua_close(L);
        return;
    }

    if (!lua_istable(L, -1)) {
        LOG_WARN("[EnemyDatabase] {} must return a table", path);
        lua_close(L);
        return;
    }

    int len = static_cast<int>(lua_rawlen(L, -1));
    m_enemies.reserve(static_cast<size_t>(len));

    for (int i = 1; i <= len; ++i) {
        lua_rawgeti(L, -1, i);
        if (lua_istable(L, -1)) {
            EnemyInfo info;
            info.lua_id        = luaGetString(L, "id");
            info.name          = luaGetString(L, "name");
            info.type          = parseEnemyType(luaGetString(L, "type"));
            info.base_hp       = luaGetInt(L, "base_hp", 10);
            info.hp_per_level  = luaGetInt(L, "hp_per_level", 2);
            info.passive_regen = static_cast<int8_t>(luaGetInt(L, "passive_regen"));
            info.is_boss       = luaGetBool(L, "is_boss");
            info.patterns      = parsePatterns(L);
            m_enemies.push_back(std::move(info));
        }
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
    lua_close(L);

    LOG_TRACE("[EnemyDatabase] Loaded {} enemy types from {}", m_enemies.size(), path);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Queries
// ─────────────────────────────────────────────────────────────────────────────

const EnemyInfo* EnemyDatabase::findByType(EnemyType type) const noexcept
{
    for (const auto& e : m_enemies)
        if (e.type == type) return &e;
    return nullptr;
}

const EnemyInfo* EnemyDatabase::findById(const std::string& lua_id) const noexcept
{
    for (const auto& e : m_enemies)
        if (e.lua_id == lua_id) return &e;
    return nullptr;
}

} // namespace battle
