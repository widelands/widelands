dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   msgctxt = "frisians_building",
   name = "frisians_outpost",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Outpost"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 5,
      granite = 1,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_?.png"),
         hotspot = {50, 78},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {50, 78},
      },
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true
   },

   max_soldiers = 6,
   heal_per_second = 160,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your outpost.",
      aggressor = _"Your outpost discovered an aggressor.",
      attack = _"Your outpost is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the outpost.",
      defeated_you = _"Your soldiers defeated the enemy at the outpost."
   },
}
