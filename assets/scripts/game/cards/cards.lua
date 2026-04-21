-- Card manifest — index order must match card IDs used in buildStarterDeck.
-- All mechanics fields are read by CardDatabase and used by BattleSystem.

return {
    -- 0
    {
        id          = "moonlight_strike",
        name        = "Moonlight Strike",
        description = "Deal 6 damage to an enemy.",
        cost        = 1,
        type        = "attack",
        art         = "card_dark",
        damage      = 6,
    },
    -- 1
    {
        id          = "shield_bash",
        name        = "Shield Bash",
        description = "Gain 4 Block. Deal 3 damage.",
        cost        = 1,
        type        = "attack",
        art         = "card_dark",
        damage      = 3,
        block       = 4,
    },
    -- 2
    {
        id          = "lunar_heal",
        name        = "Lunar Heal",
        description = "Restore 8 HP.",
        cost        = 2,
        type        = "skill",
        art         = "card_skill",
        heal        = 8,
    },
    -- 3
    {
        id          = "shadow_step",
        name        = "Shadow Step",
        description = "Gain 6 Block. Apply 1 Weak to enemy.",
        cost        = 1,
        type        = "skill",
        art         = "card_skill",
        block       = 6,
        apply_weak  = 1,
    },
    -- 4
    {
        id          = "cleave",
        name        = "Cleave",
        description = "Deal 12 damage to an enemy.",
        cost        = 2,
        type        = "attack",
        art         = "card_dark",
        damage      = 12,
    },
    -- 5
    {
        id          = "iron_wall",
        name        = "Iron Wall",
        description = "Gain 8 Block.",
        cost        = 1,
        type        = "skill",
        art         = "card_skill",
        block       = 8,
    },
    -- 6
    {
        id             = "war_cry",
        name           = "War Cry",
        description    = "Gain 3 Strength this combat. Exhaust.",
        cost           = 2,
        type           = "power",
        art            = "card_white",
        apply_strength = 3,
        exhaust        = true,
    },
    -- 7
    {
        id               = "bash",
        name             = "Bash",
        description      = "Deal 8 damage. Apply 1 Vulnerable.",
        cost             = 2,
        type             = "attack",
        art              = "card_dark",
        damage           = 8,
        apply_vulnerable = 1,
    },
    {
        id             = "phoom_cry",
        name           = "Phoom Cry",
        description    = "Gain 1 Strength this combat. Exhaust.",
        cost           = 1,
        type           = "power",
        art            = "card_white",
        apply_strength = 1,
        exhaust        = true,
    },
    {
        id             = "basic_noob",
        name           = "Basic Noob",
        description    = "Gain -1 Strength this combat. Exhaust.",
        cost           = 1,
        type           = "power",
        art            = "card_white",
        apply_strength = -1,
        exhaust        = true,
    },
}
