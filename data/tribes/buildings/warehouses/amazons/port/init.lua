push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_warehouse_type {
   name = "amazons_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Port"),
   icon = dirname .. "menu.png",
   size = "port",
   map_check = {"seafaring"},

   buildcost = {
      balsa = 3,
      granite = 3,
      log = 4,
      rubber = 3,
      gold = 2,
      rope = 3
   },
   return_on_dismantle = {
      balsa = 2,
      granite = 1,
      log = 2,
      rubber = 1,
      gold = 1,
      rope = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {90, 84}},
      build = {hotspot = {90, 84}},
   },

   aihints = {
      prohibited_till = 1000
   },

   conquers = 5,
   heal_per_second = 170,
}

pop_textdomain()
