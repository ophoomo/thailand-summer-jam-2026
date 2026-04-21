-- Enemy manifest — each entry defines one enemy type.
--
-- Fields:
--   id            (string)  unique key used by C++ lookups
--   name          (string)  display name shown in UI
--   type          (string)  must match EnemyType enum:
--                             "skeleton" | "goblin" | "troll" | "archer"
--                             "dark_knight" | "necromancer"
--                             "boss_dragon" | "boss_lich"
--   base_hp       (int)     HP at level 1
--   hp_per_level  (int)     extra HP added per level above 1
--   passive_regen (int)     HP healed at end of every turn (0 = none)
--   is_boss       (bool)    true for level-10 bosses
--   patterns      (table)   action cycle, looped with modulo
--
-- Pattern fields:
--   action            (string)  "attack" | "defend" | "buff" | "debuff" | "special"
--   damage            (int)     base damage (scaled +5% per level in C++)
--   block             (int)     block gained when action == "defend"
--   times             (int)     multi-hit count  (default 1)
--   apply_vulnerable  (int)     stacks of Vulnerable applied to player
--   apply_weak        (int)     stacks of Weak applied to player

return {

    -- ── Tier 1  (levels 1-3) ────────────────────────────────────────────────

    {
        id           = "skeleton",
        name         = "Skeleton",
        type         = "skeleton",
        base_hp      = 18,
        hp_per_level = 3,
        patterns     = {
            { action = "attack", damage = 6 },
            { action = "attack", damage = 6 },
            { action = "defend", block  = 4 },
        },
    },

    {
        id           = "goblin",
        name         = "Goblin",
        type         = "goblin",
        base_hp      = 14,
        hp_per_level = 2,
        patterns     = {
            { action = "debuff", apply_weak = 1 },
            { action = "attack", damage = 5 },
            { action = "attack", damage = 5 },
            { action = "attack", damage = 7 },
        },
    },

    -- ── Tier 2  (levels 4-6) ────────────────────────────────────────────────

    {
        id            = "troll",
        name          = "Troll",
        type          = "troll",
        base_hp       = 40,
        hp_per_level  = 5,
        passive_regen = 2,
        patterns      = {
            { action = "attack", damage = 9  },
            { action = "defend", block  = 8  },
            { action = "buff"                },
            { action = "attack", damage = 12 },
        },
    },

    {
        id           = "archer",
        name         = "Archer",
        type         = "archer",
        base_hp      = 22,
        hp_per_level = 3,
        patterns     = {
            { action = "attack",  damage = 7, apply_vulnerable = 1 },
            { action = "attack",  damage = 7  },
            { action = "debuff",  apply_vulnerable = 1 },
            { action = "attack",  damage = 10 },
        },
    },

    -- ── Tier 3  (levels 7-9) ────────────────────────────────────────────────

    {
        id           = "dark_knight",
        name         = "Dark Knight",
        type         = "dark_knight",
        base_hp      = 50,
        hp_per_level = 6,
        patterns     = {
            { action = "defend", block  = 10 },
            { action = "attack", damage = 12 },
            { action = "attack", damage = 12 },
            { action = "attack", damage = 16 },
        },
    },

    {
        id           = "necromancer",
        name         = "Necromancer",
        type         = "necromancer",
        base_hp      = 35,
        hp_per_level = 4,
        patterns     = {
            { action = "special"             },  -- summon skeleton
            { action = "attack", damage = 8  },
            { action = "defend", block  = 12 },
            { action = "attack", damage = 10 },
        },
    },

    -- ── Level 10 bosses ──────────────────────────────────────────────────────

    {
        id           = "boss_dragon",
        name         = "Dragon",
        type         = "boss_dragon",
        base_hp      = 180,
        hp_per_level = 20,
        is_boss      = true,
        patterns     = {
            { action = "special", damage = 16 },        -- fire breath
            { action = "attack",  damage = 20 },
            { action = "defend",  block  = 16 },
            { action = "attack",  damage = 25 },
            { action = "special", damage = 12, times = 2 },  -- double hit
        },
    },

    {
        id           = "boss_lich",
        name         = "Lich",
        type         = "boss_lich",
        base_hp      = 160,
        hp_per_level = 18,
        is_boss      = true,
        patterns     = {
            { action = "debuff",  apply_vulnerable = 1, apply_weak = 1 },
            { action = "attack",  damage = 14 },
            { action = "buff"                  },
            { action = "attack",  damage = 18 },
            { action = "special", damage = 20 },  -- life drain
        },
    },
}
