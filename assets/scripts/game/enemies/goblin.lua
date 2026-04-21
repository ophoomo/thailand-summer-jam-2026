return {
  id       = "goblin",
  name     = "Goblin",
  max_hp   = 20,
  attack   = 6,
  defense  = 0,

  on_turn  = function()
    Battle.enemy_attack(6)
  end,

  on_death = function()
    -- reward hooks go here
  end,
}
