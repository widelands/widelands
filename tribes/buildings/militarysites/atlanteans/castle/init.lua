dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Castle"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 4,
      granite = 7,
      log = 5,
      spidercloth = 2,
      diamond = 1,
      quartz = 1
   },
   return_on_dismantle = {
      planks = 2,
      granite = 5,
      log = 2
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 91, 91 },
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 1500
   },

   max_soldiers = 12,
   heal_per_second = 200,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your castle.",
      aggressor = _"Your castle discovered an aggressor.",
      attack = _"Your castle is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the castle.",
      defeated_you = _"Your soldiers defeated the enemy at the castle."
   },
}
