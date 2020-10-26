push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "barbarians_citadel",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Citadel"),
   icon = dirname .. "menu.png",
   size = "big",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 102, 102 },
         fps = 10
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 102, 102 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 102, 102 }
      }
   },

   aihints = {},

   max_soldiers = 12,
   heal_per_second = 220,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your citadel.",
      aggressor = _"Your citadel discovered an aggressor.",
      attack = _"Your citadel is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the citadel.",
      defeated_you = _"Your soldiers defeated the enemy at the citadel."
   },
}

pop_textdomain()
