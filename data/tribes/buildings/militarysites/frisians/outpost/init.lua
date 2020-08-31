push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_militarysite_type {
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
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {45, 68},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {45, 68}
      }
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

pop_textdomain()
