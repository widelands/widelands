push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_militarysite_type {
   name = "frisians_wooden_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Wooden Tower"),
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 14,

   enhancement = {
      name = "frisians_wooden_tower_high",
      enhancement_cost = {
         log = 2,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         log = 1,
      }
   },

   buildcost = {
      log = 3,
      reed = 1
   },
   return_on_dismantle = {
      log = 2
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {22, 69},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {22, 69}
      }
   },

   aihints = {
      expansion = true,
      prohibited_till = 400
   },

   max_soldiers = 1,
   heal_per_second = 40,
   conquers = 5,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your wooden tower.",
      aggressor = _"Your wooden tower discovered an aggressor.",
      attack = _"Your wooden tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the wooden tower.",
      defeated_you = _"Your soldiers defeated the enemy at the wooden tower."
   },
}

pop_textdomain()
