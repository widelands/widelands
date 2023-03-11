push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_militarysite_type {
   name = "frisians_sentinel",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Sentinel"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 8,

   buildcost = {
      brick = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   spritesheets = {
      idle = {
         hotspot = {28, 45},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {28, 45}
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
   },

   max_soldiers = 2,
   heal_per_second = 100,
   conquers = 6,
   prefer_heroes = false,

   messages = {
      -- TRANSLATORS: Message sent by a Frisian military site
      occupied = pgettext("frisians_building", "Your soldiers have occupied your sentinel."),
      -- TRANSLATORS: Message sent by a Frisian military site
      aggressor = pgettext("frisians_building", "Your sentinel discovered an aggressor."),
      -- TRANSLATORS: Message sent by a Frisian military site
      attack = pgettext("frisians_building", "Your sentinel is under attack."),
      -- TRANSLATORS: Message sent by a Frisian military site
      defeated_enemy = pgettext("frisians_building", "The enemy defeated your soldiers at the sentinel."),
      -- TRANSLATORS: Message sent by a Frisian military site
      defeated_you = pgettext("frisians_building", "Your soldiers defeated the enemy at the sentinel.")
   },
}

pop_textdomain()
