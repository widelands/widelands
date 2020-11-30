push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   msgctxt = "europeans_building",
   name = "europeans_blockhouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Blockhouse"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 14,
   
   enhancement = {
        name = "europeans_sentry",
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
         hotspot = { 37, 58 }
      }
   },

   aihints = {
      fighting = true,
      mountain_conqueror = true
   },

   max_soldiers = 3,
   heal_per_second = 90,
   conquers = 6,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Europeans military site
      occupied = pgettext("europeans_building", "Your soldiers have occupied your blockhouse."),
      -- TRANSLATORS: Message sent by an Europeans military site
      aggressor = pgettext("europeans_building", "Your blockhouse discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Europeans military site
      attack = pgettext("europeans_building", "Your blockhouse is under attack."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_enemy = pgettext("europeans_building", "The enemy defeated your soldiers at the blockhouse."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_you = pgettext("europeans_building", "Your soldiers defeated the enemy at the blockhouse.")
   },
}

pop_textdomain()
