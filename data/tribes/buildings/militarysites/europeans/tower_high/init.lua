push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   msgctxt = "europeans_building",
   name = "europeans_tower_high",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "High Tower"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 20,
   
   enhancement = {
        name = "europeans_advanced_tower",
        enhancement_cost = {
            planks = 3,
            brick = 2,
            grout = 2,
            marble_column = 3,
            quartz = 2,
            diamond = 2
        },
        enhancement_return_on_dismantle = {
            log = 2,
            granite = 2,
            marble = 2,
            quartz = 2,
            diamond = 2
        },
   },
   
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 47, 78 },
      }
   },

   aihints = {
     fighting = true,
      mountain_conqueror = true,
      prohibited_till = 3600
   },

   max_soldiers = 6,
   heal_per_second = 170,
   conquers = 12,
   prefer_heroes = true,

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
