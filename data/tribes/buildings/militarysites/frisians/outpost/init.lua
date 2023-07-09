push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_militarysite_type {
   name = "frisians_outpost",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Outpost"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 5,
      granite = 1,
      log = 1,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {45, 68},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {45, 68}
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true
   },

   max_soldiers = 6,
   heal_per_second = 160,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by a Frisian military site
      occupied = pgettext("frisians_building", "Your soldiers have occupied your outpost."),
      -- TRANSLATORS: Message sent by a Frisian military site
      aggressor = pgettext("frisians_building", "Your outpost discovered an aggressor."),
      -- TRANSLATORS: Message sent by a Frisian military site
      attack = pgettext("frisians_building", "Your outpost is under attack."),
      -- TRANSLATORS: Message sent by a Frisian military site
      defeated_enemy = pgettext("frisians_building", "The enemy defeated your soldiers at the outpost."),
      -- TRANSLATORS: Message sent by a Frisian military site
      defeated_you = pgettext("frisians_building", "Your soldiers defeated the enemy at the outpost.")
   },
}

pop_textdomain()
