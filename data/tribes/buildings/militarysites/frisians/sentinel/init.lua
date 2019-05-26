dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   msgctxt = "frisians_building",
   name = "frisians_sentinel",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Sentinel"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 8,

   buildcost = {
      brick = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_?.png"),
         hotspot = {33, 53},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {33, 53},
      },
   },

   aihints = {
      expansion = true,
      fighting = true,
   },

   max_soldiers = 2,
   heal_per_second = 100,
   conquers = 6,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your sentinel.",
      aggressor = _"Your sentinel discovered an aggressor.",
      attack = _"Your sentinel is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the sentinel.",
      defeated_you = _"Your soldiers defeated the enemy at the sentinel."
   },
}
