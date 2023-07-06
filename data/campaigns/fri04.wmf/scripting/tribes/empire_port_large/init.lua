push_textdomain("scenario_fri04.wmf")

local dirname = "campaigns/fri04.wmf/" .. path.dirname (__file__)

wl.Descriptions():new_warehouse_type {
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
         columns = 10,
         rows = 2,
         hotspot = { 87, 116 }
      },
   },

   aihints = {},

   conquers = 11,
   heal_per_second = 170,
}

pop_textdomain()
