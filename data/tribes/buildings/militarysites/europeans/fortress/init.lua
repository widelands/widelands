push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "europeans_fortress",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Fortress"),
   icon = dirname .. "menu.png",
   size = "big",

   enhancement = {
        name = "europeans_advanced_castle",
        enhancement_cost = {
            planks = 3,
            brick = 3,
            grout = 3,
            marble_column = 3,
            quartz = 3,
            diamond = 3
        },
        enhancement_return_on_dismantle = {
            log = 3,
            granite = 3,
            marble = 3,
            quartz = 3,
            diamond = 3
        },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 90, 105 }
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 90, 105 },
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 9600
   },

   max_soldiers = 18,
   heal_per_second = 240,
   conquers = 18 ,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Europeans military site
      occupied = pgettext("europeans_building", "Your soldiers have occupied your fortress."),
      -- TRANSLATORS: Message sent by an Europeans military site
      aggressor = pgettext("europeans_building", "Your fortress discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Europeans military site
      attack = pgettext("europeans_building", "Your fortress is under attack."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_enemy = pgettext("europeans_building", "The enemy defeated your soldiers at the fortress."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_you = pgettext("europeans_building", "Your soldiers defeated the enemy at the fortress.")
   },
}

pop_textdomain()

