dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   msgctxt = "frisians_building",
   name = "frisians_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Warehouse"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   representative_image = dirname .. "representative_image.png",
   size = "medium",

   buildcost = {
      brick = 10,
      log = 1,
      thatch_reed = 6
   },
   return_on_dismantle = {
      brick = 5,
      thatch_reed = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 60, 78 },
         scale = 3.62
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 60, 78 },
      }
   },

   aihints = {},

   heal_per_second = 170,
}
