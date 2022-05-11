push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_warehouse_type {
   name = "barbarians_headquarters_interim",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Headquarters"),
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   -- The interim headquarters of the barbarians is a simple stone building thatched
   -- with reed and coated with many different wares.
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 64, 106 }
      },
   },

   aihints = {},

   heal_per_second = 170,
   conquers = 9,
}

pop_textdomain()
