dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "frisians_building",
   name = "frisians_fortress",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Fortress"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 10,
      granite = 4,
      log = 3,
      thatch_reed = 5
   },
   return_on_dismantle = {
      brick = 5,
      granite = 2,
      log = 2,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 91, 106 }
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 1200
   },

   max_soldiers = 10,
   heal_per_second = 260,
   conquers = 14,
   prefer_heroes = true,

   messages = {
      occupied = _"Your warriors have occupied your fortress.",
      aggressor = _"Your fortress discovered an aggressor.",
      attack = _"Your fortress is under attack.",
      defeated_enemy = _"The enemy defeated your warriors at the fortress.",
      defeated_you = _"Your warriors defeated the enemy at the fortress."
   },
}
