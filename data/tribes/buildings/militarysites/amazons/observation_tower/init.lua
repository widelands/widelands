push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_militarysite_type {
   name = "amazons_observation_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Observation Tower"),
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 19,

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {50, 94}},
      unoccupied = {hotspot = {50, 94}},
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

pop_textdomain()
