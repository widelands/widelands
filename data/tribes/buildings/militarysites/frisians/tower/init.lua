push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_militarysite_type {
   name = "frisians_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Tower"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",
   vision_range = 21,

   buildcost = {
      brick = 6,
      granite = 2,
      log = 4,
      reed = 3
   },
   return_on_dismantle = {
      brick = 3,
      granite = 1,
      log = 2,
      reed = 1

   },

   spritesheets = {
      idle = {
         hotspot = {64, 94},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {64, 94}
      }
   },

   aihints = {
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 500
   },

   max_soldiers = 7,
   heal_per_second = 170,
   conquers = 10,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by a Frisian military site
      occupied = pgettext("frisians_building", "Your soldiers have occupied your tower."),
      -- TRANSLATORS: Message sent by a Frisian military site
      aggressor = pgettext("frisians_building", "Your tower discovered an aggressor."),
      -- TRANSLATORS: Message sent by a Frisian military site
      attack = pgettext("frisians_building", "Your tower is under attack."),
      -- TRANSLATORS: Message sent by a Frisian military site
      defeated_enemy = pgettext("frisians_building", "The enemy defeated your soldiers at the tower."),
      -- TRANSLATORS: Message sent by a Frisian military site
      defeated_you = pgettext("frisians_building", "Your soldiers defeated the enemy at the tower.")
   },
}

pop_textdomain()
