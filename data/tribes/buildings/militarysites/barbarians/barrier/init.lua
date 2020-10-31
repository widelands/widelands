push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "barbarians_barrier",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Barrier"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      blackwood = 5,
      grout = 2
   },
   return_on_dismantle = {
      blackwood = 2,
      grout = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 44, 62 },
         fps = 10
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 44, 62 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 44, 62 },
      }
   },

   aihints = {},

   max_soldiers = 5,
   heal_per_second = 130,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your barrier.",
      aggressor = _"Your barrier discovered an aggressor.",
      attack = _"Your barrier is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the barrier.",
      defeated_you = _"Your soldiers defeated the enemy at the barrier."
   },
}

pop_textdomain()
