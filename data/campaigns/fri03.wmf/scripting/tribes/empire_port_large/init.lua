push_textdomain("tribes")

dirname = "tribes/buildings/warehouses/empire/port/"

descriptions:new_warehouse_type {
   name = "empire_port_large",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Port"),
   icon = dirname .. "menu.png",
   size = "port",
   destructible = false,
   map_check = {"seafaring"},

   animation_directory = dirname,
   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 74, 96 }
      },
   },

   aihints = {},

   conquers = 11,
   heal_per_second = 170,
}

pop_textdomain()
