push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_warehouse_type {
   msgctxt = "europeans_building",
   name = "europeans_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Port"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "port",
   map_check = {"seafaring"},

   buildcost = {
      planks = 6,
      reed = 2,
      spidercloth = 2,
      brick = 3,
      grout = 3,
      quartz = 3,
      diamond = 3
   },
   return_on_dismantle = {
      log = 4,
      reed = 1,
      spidercloth = 1,
      granite = 2,
      quartz = 2,
      diamond = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 74, 70 },
         fps = 10
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 74, 70 },
      }
   },

   aihints = {
   },

   conquers = 5,
   heal_per_second = 170,
}

pop_textdomain()
