push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_warehouse_type {
   msgctxt = "europeans_building",
   name = "europeans_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Headquarters"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   destructible = true,

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 81, 110 },
      },
   },

   buildcost = {
      planks = 12,
      spidercloth = 4,
      reed = 4,
      brick = 4,
      grout = 4,
      marble_column = 4,
      quartz = 3,
      diamond = 3
   },
   return_on_dismantle = {
      log = 6,
      granite = 4,
      marble = 2,
      quartz = 2,
      diamond = 2
   },


   aihints = {
      prohibited_till = 18000
   },

   heal_per_second = 220,
   conquers = 16,
}

pop_textdomain()
