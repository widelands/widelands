push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "empire_barrier",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Barrier"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      planks = 2,
      granite = 2,
      marble = 1
   },
   return_on_dismantle = {
      log = 1,
      planks = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 49, 77 }
      }
   },

   aihints = {
      fighting = true
   },

   max_soldiers = 5,
   heal_per_second = 130,
   conquers = 8,
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
