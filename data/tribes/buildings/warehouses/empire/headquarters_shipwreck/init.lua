push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_warehouse_type {
   name = "empire_headquarters_shipwreck",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Headquarters Shipwreck"),
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 93, 40 },
      },
   },

   aihints = {},

   heal_per_second = 170,
   conquers = 9,
}

pop_textdomain()
