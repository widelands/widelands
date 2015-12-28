dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Tower"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 17,
   enhancement = "atlanteans_tower_high",

   buildcost = {
      log = 2,
      planks = 3,
      granite = 4,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 1,
      planks = 1,
      granite = 3
   },

   aihints = {
      expansion = true,
      mountain_conqueror = true,
      prohibited_till = 600
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 55, 60 },
      }
   },

   max_soldiers = 4,
   heal_per_second = 120,
   conquers = 9,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your tower.",
      aggressor = _"Your tower discovered an aggressor.",
      attack = _"Your tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the tower.",
      defeated_you = _"Your soldiers defeated the enemy at the tower."
   },
}
