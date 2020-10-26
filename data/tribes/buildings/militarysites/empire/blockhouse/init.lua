push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "empire_blockhouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Blockhouse"),
   icon = dirname .. "menu.png",
   size = "small",

   enhancement = {
      name = "empire_sentry",
      enhancement_cost = {
         planks = 1,
         granite = 1
      },
      enhancement_return_on_dismantle = {
         granite = 1
      }
   },

   buildcost = {
      log = 1,
      planks = 2
   },
   return_on_dismantle = {
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 37, 58 }
      }
   },

   aihints = {},

   max_soldiers = 1,
   heal_per_second = 60, -- very low -> smallest building
   conquers = 6,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your blockhouse.",
      aggressor = _"Your blockhouse discovered an aggressor.",
      attack = _"Your blockhouse is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the blockhouse.",
      defeated_you = _"Your soldiers defeated the enemy at the blockhouse."
   },
}

pop_textdomain()
