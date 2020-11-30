push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   msgctxt = "europeans_building",
   name = "europeans_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Medium Tower"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 16,

   enhancement = {
        name = "europeans_tower_high",
        enhancement_cost = {
            planks = 2,
            brick = 2,
            grout = 2,
            marble_column = 1,
            quartz = 1,
            diamond = 1
        },
        enhancement_return_on_dismantle = {
            log = 2,
            granite = 2,
            marble = 1,
            quartz = 1,
            diamond = 1
        },
   },

   buildcost = {
      planks = 4,
      spidercloth = 4,
      brick = 4,
      grout = 4
   },
   return_on_dismantle = {
      log = 2,
      spidercloth = 1,
      granite = 3
   },

   aihints = {
      expansion = true,
      mountain_conqueror = true,
      prohibited_till = 1800
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 46, 58 },
      }
   },

   max_soldiers = 4,
   heal_per_second = 120,
   conquers = 8,
   prefer_heroes = false,

   messages = {
      -- TRANSLATORS: Message sent by an Europeans military site
      occupied = pgettext("europeans_building", "Your soldiers have occupied your tower."),
      -- TRANSLATORS: Message sent by an Europeans military site
      aggressor = pgettext("europeans_building", "Your tower discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Europeans military site
      attack = pgettext("europeans_building", "Your tower is under attack."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_enemy = pgettext("europeans_building", "The enemy defeated your soldiers at the tower."),
      -- TRANSLATORS: Message sent by an Europeans military site
      defeated_you = pgettext("europeans_building", "Your soldiers defeated the enemy at the tower.")
   },
}

pop_textdomain()
