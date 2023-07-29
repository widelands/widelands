push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_warehouse_type {
   name = "barbarians_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Headquarters"),
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   -- The Headquarters of the Barbarians is from the apperance a stable military
   -- Building, fortified base and some towers made of blackwood. Some flags
   -- in the player colors may be present
   animation_directory = dirname,
   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 90, 96 }
      },
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}

pop_textdomain()
