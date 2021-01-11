push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "empire_sentry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Sentry"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      planks = 1,
      log = 1,
      granite = 1
   },
   return_on_dismantle = {
      granite = 1,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 37, 60 }
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true
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

pop_textdomain()
