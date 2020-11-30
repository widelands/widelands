push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "europeans_tower_small",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Small Tower"),
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 12,

   enhancement = {
        name = "europeans_blockhouse",
        enhancement_cost = {
            planks = 1,
            brick = 1,
            grout = 1,
            quartz = 1,
            diamond = 1
        },
        enhancement_return_on_dismantle = {
            log = 1,
            granite = 1,
            quartz = 1,
            diamond = 1
        },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 65 },
      }
   },

   aihints = {
      fighting = true,
      expansion = true,
      mountain_conqueror = true
   },

   max_soldiers = 2,
   heal_per_second = 75,
   conquers = 5,
   prefer_heroes = false,

   messages = {
      -- TRANSLATORS: Message sent by an Europeans military site
      occupied = pgettext("europeans_building", "Your soldiers have occupied your small tower."),
      -- TRANSLATORS: Message sent by an Europeans military site
      aggressor = pgettext("europeans_building", "Your small tower discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Europeans military site
      attack = pgettext("europeans_building", "Your small tower is under attack."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_enemy = pgettext("europeans_building", "The enemy defeated your soldiers at the small tower."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_you = pgettext("europeans_building", "Your soldiers defeated the enemy at the small tower.")
   },
}

pop_textdomain()
