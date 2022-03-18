push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_militarysite_type {
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

   animation_directory = dirname,
   animations = {
      idle = {
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
      -- TRANSLATORS: Message sent by an Empire military site
      occupied = pgettext("empire_building", "Your soldiers have occupied your outpost."),
      -- TRANSLATORS: Message sent by an Empire military site
      aggressor = pgettext("empire_building", "Your outpost discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Empire military site
      attack = pgettext("empire_building", "Your outpost is under attack."),
      -- TRANSLATORS: Message sent by an Empire military site
      defeated_enemy = pgettext("empire_building", "The enemy defeated your soldiers at the outpost."),
      -- TRANSLATORS: Message sent by an Empire military site
      defeated_you = pgettext("empire_building", "Your soldiers defeated the enemy at the outpost.")
   },
}

pop_textdomain()
