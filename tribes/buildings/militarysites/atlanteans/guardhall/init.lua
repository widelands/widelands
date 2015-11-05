dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_guardhall",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Guardhall"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      planks = 3,
      granite = 4,
      diamond = 1
   },
   return_on_dismantle = {
      log = 1,
      planks = 1,
      granite = 3
   },

   aihints = {
      fighting = true
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 58, 72 },
      }
   },

   outputs = {
      "atlanteans_soldier",
   },

   max_soldiers = 7,
   heal_per_second = 140,
   conquers = 7,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your guardhall.",
      aggressor = _"Your guardhall discovered an aggressor.",
      attack = _"Your guardhall is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the guardhall.",
      defeated_you = _"Your soldiers defeated the enemy at the guardhall."
   },
}
