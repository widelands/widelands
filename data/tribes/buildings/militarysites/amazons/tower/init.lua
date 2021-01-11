push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_militarysite_type {
   name = "amazons_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Tower"),
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 16,

   enhancement = {
      name = "amazons_observation_tower",
      enhancement_cost = {
         ironwood = 1,
         rope = 1,
         balsa = 2
      },
      enhancement_return_on_dismantle = {
         ironwood = 1,
         balsa = 1
      }
   },

   buildcost = {
      rope = 2,
      granite = 2,
      log = 2,
      ironwood = 2
   },
   return_on_dismantle = {
      rope = 1,
      granite = 1,
      log = 1,
      ironwood = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {50, 81}},
      unoccupied = {hotspot = {50, 81}},
   },

   aihints = {
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 500
   },

   max_soldiers = 4,
   heal_per_second = 110,
   conquers = 8,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your tower.",
      aggressor = _"Your tower discovered an aggressor.",
      attack = _"Your tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the tower.",
      defeated_you = _"Your soldiers defeated the enemy at the tower."
   },
}

pop_textdomain()
