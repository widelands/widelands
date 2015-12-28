dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_tower_high",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "High Tower"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 21,

   enhancement_cost = {
      log = 1,
      planks = 1,
      granite = 2
   },
   return_on_dismantle_on_enhanced = {
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 73 },
      }
   },

   aihints = {},

   outputs = {
      "atlanteans_soldier",
   },

   max_soldiers = 5,
   heal_per_second = 170,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your high tower.",
      aggressor = _"Your high tower discovered an aggressor.",
      attack = _"Your high tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the high tower.",
      defeated_you = _"Your soldiers defeated the enemy at the high tower."
   },
}
