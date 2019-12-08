dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   msgctxt = "frisians_building",
   name = "frisians_wooden_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Wooden Tower"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 14,
   enhancement = "frisians_wooden_tower_high",

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
