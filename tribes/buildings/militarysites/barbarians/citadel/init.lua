dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_citadel",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Citadel"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   enhancement_cost = {
      blackwood = 9,
      log = 5,
      granite = 4,
      grout = 2
   },
   return_on_dismantle_on_enhanced = {
      blackwood = 4,
      log = 2,
      granite = 3,
      grout = 1
   },

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

   outputs = {
      "barbarians_soldier",
   },

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
