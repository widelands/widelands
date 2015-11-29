dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "empire_building",
   name = "empire_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Tower"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 19,

   buildcost = {
      log = 2,
      planks = 3,
      granite = 4,
      marble_column = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 2,
      marble_column = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 53, 81 }
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 53, 81 },
      }
   },

   aihints = {
      mountain_conqueror = true,
      prohibited_till = 300
   },

   max_soldiers = 5,
   heal_per_second = 150,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your tower.",
      aggressor = _"Your tower discovered an aggressor.",
      attack = _"Your tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the tower.",
      defeated_you = _"Your soldiers defeated the enemy at the tower."
   },
}
