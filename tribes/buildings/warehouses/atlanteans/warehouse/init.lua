dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Warehouse"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      planks = 2,
      granite = 2,
      quartz = 1,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 1,
      planks = 1,
      granite = 1,
      quartz = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 58, 62 }
      }
   },

   aihints = {},

   heal_per_second = 170,
}