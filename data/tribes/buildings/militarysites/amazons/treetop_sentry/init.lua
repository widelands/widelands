push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   name = "amazons_treetop_sentry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Treetop Sentry"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   built_over_immovable = "tree",
   vision_range = 9,


   buildcost = {
      log = 1,
      rope = 1
   },
   return_on_dismantle = {
      rope = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_?.png"),
         hotspot = {31, 89},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {31, 89},
      },
   },

   aihints = {
      expansion = true,
      prohibited_till = 400
   },

   max_soldiers = 1,
   heal_per_second = 50,
   conquers = 5,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your treetop sentry.",
      aggressor = _"Your treetop sentry discovered an aggressor.",
      attack = _"Your treetop sentry is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the treetop sentry.",
      defeated_you = _"Your soldiers defeated the enemy at the treetop sentry."
   },
}

pop_textdomain()
