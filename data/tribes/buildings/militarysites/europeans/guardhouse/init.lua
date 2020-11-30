push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "europeans_guardhouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Guardhouse"),
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 10,
   
   enhancement = {
        name = "europeans_tower_small",
        enhancement_cost = {
            planks = 1,
            brick = 1,
            grout = 1,
            spidercloth = 1,
            quartz = 1
        },
        enhancement_return_on_dismantle = {
            log = 1,
            granite = 1,
            quartz = 1
        },
   },

   buildcost = {
      planks = 2,
      reed = 1
   },
   return_on_dismantle = {
      log = 2
   },

   aihints = {
      expansion = true,
      mountain_conqueror = true
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 33, 41 },
      }
   },

   max_soldiers = 1,
   heal_per_second = 60,
   conquers = 5,
   prefer_heroes = false,

   messages = {
      -- TRANSLATORS: Message sent by an Europeans military site
      occupied = pgettext("europeans_building", "Your soldiers have occupied your guardhouse."),
      -- TRANSLATORS: Message sent by an Europeans military site
      aggressor = pgettext("europeans_building", "Your guardhouse discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Europeans military site
      attack = pgettext("europeans_building", "Your guardhouse is under attack."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_enemy = pgettext("europeans_building", "The enemy defeated your soldiers at the guardhouse."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_you = pgettext("europeans_building", "Your soldiers defeated the enemy at the guardhouse.")
   },
}

pop_textdomain()
