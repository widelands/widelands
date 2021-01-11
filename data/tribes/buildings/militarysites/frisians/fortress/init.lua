push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_militarysite_type {
   name = "frisians_fortress",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Fortress"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 10,
      granite = 4,
      log = 3,
      reed = 5
   },
   return_on_dismantle = {
      brick = 5,
      granite = 2,
      log = 2,
      reed = 2
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {85, 99},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {85, 99}
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 1400
   },

   max_soldiers = 12,
   heal_per_second = 220,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your fortress.",
      aggressor = _"Your fortress discovered an aggressor.",
      attack = _"Your fortress is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the fortress.",
      defeated_you = _"Your soldiers defeated the enemy at the fortress."
   },
}

pop_textdomain()
