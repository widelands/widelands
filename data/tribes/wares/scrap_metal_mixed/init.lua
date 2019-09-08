dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "scrap_metal_mixed",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Mixed Scrap Metal"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      frisians = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 10 },
      },
   }
}
