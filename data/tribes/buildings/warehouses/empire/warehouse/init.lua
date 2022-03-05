push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_warehouse_type {
   name = "empire_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Warehouse"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      planks = 2,
      granite = 2,
      marble = 3,
      marble_column = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 2,
      marble = 2,
      marble_column = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 58, 55 }
      }
   },

   aihints = {},

   heal_per_second = 170,
}

pop_textdomain()
