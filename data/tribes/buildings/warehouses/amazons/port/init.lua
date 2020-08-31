push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_warehouse_type {
   name = "amazons_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Port"),
   helptext_script = dirname .. "helptexts.lua",
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

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {95, 178},
         fps = 10,
      },
      build = {
         pictures = path.list_files (dirname .. "build_?.png"),
         hotspot = {95, 178},
      },
   },

   aihints = {
      prohibited_till = 1000
   },

   conquers = 5,
   heal_per_second = 170,
}

pop_textdomain()
