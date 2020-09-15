push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   name = "amazons_fortress",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Fortress"),
   icon = dirname .. "menu.png",
   size = "big",
   vision_range = 15,
   enhancement = "amazons_fortification",

   buildcost = {
      ironwood = 6,
      granite = 6,
      log = 2,
      rubber = 2
   },
   return_on_dismantle = {
      ironwood = 3,
      granite = 3,
      log = 1,
      rubber = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {93, 110},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {93, 110},
      },
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 1400
   },

   max_soldiers = 8,
   heal_per_second = 170,
   conquers = 11,
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
