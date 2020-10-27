push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_warehouse_type {
   name = "barbarians_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Warehouse"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 2,
      grout = 3,
      reed = 1
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 1,
      grout = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 60, 78 },
      },
      build = {
         hotspot = { 60, 78 },
      }
   },

   aihints = {},

   heal_per_second = 170,
}

pop_textdomain()
