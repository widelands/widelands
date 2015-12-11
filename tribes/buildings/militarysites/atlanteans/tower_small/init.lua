dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_tower_small",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Small Tower"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 13,

   buildcost = {
      log = 1,
      planks = 2,
      granite = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 65 },
      }
   },

   aihints = {
      fighting = true
   },

   outputs = {
      "atlanteans_soldier",
   },

   max_soldiers = 3,
   heal_per_second = 100,
   conquers = 5,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your small tower.",
      aggressor = _"Your small tower discovered an aggressor.",
      attack = _"Your small tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the small tower.",
      defeated_you = _"Your soldiers defeated the enemy at the small tower."
   },
}
