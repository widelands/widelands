push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "empire_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Tower"),
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 19,

   buildcost = {
      log = 2,
      planks = 3,
      granite = 4,
      marble_column = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 2,
      marble_column = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 81 }
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 53, 81 },
      }
   },

   aihints = {
      mountain_conqueror = true,
      prohibited_till = 400
   },

   max_soldiers = 5,
   heal_per_second = 150,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Empire military site
      occupied = pgettext("empire_building", "Your soldiers have occupied your tower."),
      -- TRANSLATORS: Message sent by an Empire military site
      aggressor = pgettext("empire_building", "Your tower discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Empire military site
      attack = pgettext("empire_building", "Your tower is under attack."),
      -- TRANSLATORS: Message sent by an Empire military site
      defeated_enemy = pgettext("empire_building", "The enemy defeated your soldiers at the tower."),
      -- TRANSLATORS: Message sent by an Empire military site
      defeated_you = pgettext("empire_building", "Your soldiers defeated the enemy at the tower.")
   },
}

pop_textdomain()
