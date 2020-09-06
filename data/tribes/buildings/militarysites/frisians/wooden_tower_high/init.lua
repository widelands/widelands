push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   name = "frisians_wooden_tower_high",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "High Wooden Tower"),
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 17,

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {22, 72},
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
         hotspot = {22, 72}
      }
   },

   aihints = {
      expansion = true,
   },

   max_soldiers = 2,
   heal_per_second = 70,
   conquers = 6,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your high wooden tower.",
      aggressor = _"Your high wooden tower discovered an aggressor.",
      attack = _"Your high wooden tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the high wooden tower.",
      defeated_you = _"Your soldiers defeated the enemy at the high wooden tower."
   },
}

pop_textdomain()
