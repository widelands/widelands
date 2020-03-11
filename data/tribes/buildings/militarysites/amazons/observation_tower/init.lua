dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   msgctxt = "amazons_building",
   name = "amazons_observation_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Observation Tower"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 19,

   enhancement_cost = {
      ironwood = 1,
      rope = 1,
      balsa = 2
   },
   return_on_dismantle_on_enhanced = {
      ironwood = 1,
      balsa = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_?.png"),
         hotspot = {31, 93},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {31, 93},
      },
   },

   aihints = {
      expansion = true,
   },

   max_soldiers = 5,
   heal_per_second = 150,
   conquers = 9,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your observation tower.",
      aggressor = _"Your observation tower discovered an aggressor.",
      attack = _"Your observation tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the observation tower.",
      defeated_you = _"Your soldiers defeated the enemy at the observation tower."
   },
}
