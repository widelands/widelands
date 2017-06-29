dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "scrap_metal_iron",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Scrap Metal (Iron)"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      frisians = 0
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 9 },
      },
   }
}
