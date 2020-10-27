push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "empire_outpost",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Outpost"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "empire_barrier",
      enhancement_cost = {
         log = 1,
         planks = 2,
         granite = 1,
         marble = 1
      },
      enhancement_return_on_dismantle = {
         planks = 1,
         granite = 1
      }
   },

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1,
      marble = 1
   },
   return_on_dismantle = {
      granite = 1,
      marble = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 77 }
      }
   },

   aihints = {
      expansion = true,
      fighting = true
   },

   max_soldiers = 3,
   heal_per_second = 100,
   conquers = 7,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your outpost.",
      aggressor = _"Your outpost discovered an aggressor.",
      attack = _"Your outpost is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the outpost.",
      defeated_you = _"Your soldiers defeated the enemy at the outpost."
   },
}

pop_textdomain()
