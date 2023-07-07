push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_warehouse_type {
   name = "empire_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Headquarters"),
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 100, 115 },
      },
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}

pop_textdomain()
