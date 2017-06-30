dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "frisians_building",
   name = "frisians_sentinel",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Sentinel"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 3,
      log = 1,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 36, 61 }
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 1200
   },

   max_soldiers = 3,
   heal_per_second = 100,
   conquers = 7,
   prefer_heroes = true,

   messages = {
      occupied = _"Your warriors have occupied your sentinel.",
      aggressor = _"Your sentinel discovered an aggressor.",
      attack = _"Your sentinel is under attack.",
      defeated_enemy = _"The enemy defeated your warriors at the sentinel.",
      defeated_you = _"Your warriors defeated the enemy at the sentinel."
   },
}
