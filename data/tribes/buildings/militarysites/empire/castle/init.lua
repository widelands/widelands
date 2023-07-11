push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_militarysite_type {
   name = "empire_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Castle"),
   icon = dirname .. "menu.png",
   size = "big",

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 98, 114 }
      }
   },

   aihints = {},

   max_soldiers = 12,
   heal_per_second = 220,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Empire military site
      occupied = pgettext("empire_building", "Your soldiers have occupied your castle."),
      -- TRANSLATORS: Message sent by an Empire military site
      aggressor = pgettext("empire_building", "Your castle discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Empire military site
      attack = pgettext("empire_building", "Your castle is under attack."),
      -- TRANSLATORS: Message sent by an Empire military site
      defeated_enemy = pgettext("empire_building", "The enemy defeated your soldiers at the castle."),
      -- TRANSLATORS: Message sent by an Empire military site
      defeated_you = pgettext("empire_building", "Your soldiers defeated the enemy at the castle.")
   },
}

pop_textdomain()
