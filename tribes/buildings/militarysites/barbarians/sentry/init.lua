dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_sentry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Sentry"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      blackwood = 2
   },
   return_on_dismantle = {
      blackwood = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 40 },
         fps = 10
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 39, 40 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 39, 40 }
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true
   },

   outputs = {
      "barbarians_soldier",
   },

   max_soldiers = 2,
   heal_per_second = 80,
   conquers = 6,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your sentry.",
      aggressor = _"Your sentry discovered an aggressor.",
      attack = _"Your sentry is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the sentry.",
      defeated_you = _"Your soldiers defeated the enemy at the sentry."
   },
}
