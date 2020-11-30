push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_warehouse_type {
   msgctxt = "europeans_building",
   name = "europeans_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Warehouse"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      planks = 6,
      reed = 2,
      spidercloth = 2,
      brick = 4,
      grout = 4,
      quartz = 1,
      diamond = 1
   },
   return_on_dismantle = {
      log = 3,
      reed = 1,
      granite = 2,
      quartz = 1,
      diamond = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 58, 62 }
      }
   },

   aihints = {
      prohibited_till = 3600
   },

   heal_per_second = 170,
}

pop_textdomain()
