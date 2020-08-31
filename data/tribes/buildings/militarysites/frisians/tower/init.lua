push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   name = "frisians_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Tower"),
   icon = dirname .. "menu.png",
   size = "big",
   vision_range = 21,

   buildcost = {
      brick = 6,
      granite = 2,
      log = 4,
      reed = 3
   },
   return_on_dismantle = {
      brick = 3,
      granite = 1,
      log = 2,
      reed = 1

   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {64, 94},
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
         hotspot = {64, 94}
      }
   },

   aihints = {
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 500
   },

   max_soldiers = 7,
   heal_per_second = 170,
   conquers = 10,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your tower.",
      aggressor = _"Your tower discovered an aggressor.",
      attack = _"Your tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the tower.",
      defeated_you = _"Your soldiers defeated the enemy at the tower."
   },
}

pop_textdomain()
