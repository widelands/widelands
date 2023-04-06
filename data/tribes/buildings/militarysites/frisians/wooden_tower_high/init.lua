push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_militarysite_type {
   name = "frisians_wooden_tower_high",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "High Wooden Tower"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 17,

   spritesheets = {
      idle = {
         hotspot = {22, 72},
         frames = 4,
         columns = 2,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {22, 72}
      }
   },

   aihints = {
      expansion = true,
   },

   max_soldiers = 2,
   heal_per_second = 70,
   conquers = 6,
   prefer_heroes = false,

   messages = {
      -- TRANSLATORS: Message sent by a Frisian military site
      occupied = pgettext("frisians_building", "Your soldiers have occupied your high wooden tower."),
      -- TRANSLATORS: Message sent by a Frisian military site
      aggressor = pgettext("frisians_building", "Your high wooden tower discovered an aggressor."),
      -- TRANSLATORS: Message sent by a Frisian military site
      attack = pgettext("frisians_building", "Your high wooden tower is under attack."),
      -- TRANSLATORS: Message sent by a Frisian military site
      defeated_enemy = pgettext("frisians_building", "The enemy defeated your soldiers at the high wooden tower."),
      -- TRANSLATORS: Message sent by a Frisian military site
      defeated_you = pgettext("frisians_building", "Your soldiers defeated the enemy at the high wooden tower.")
   },
}

pop_textdomain()
