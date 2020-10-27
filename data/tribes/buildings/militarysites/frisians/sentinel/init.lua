push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_militarysite_type {
   name = "frisians_sentinel",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Sentinel"),
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

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {28, 45},
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
         hotspot = {28, 45}
      }
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

pop_textdomain()
