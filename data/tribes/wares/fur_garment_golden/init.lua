dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fur_garment_golden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Golden Fur Garment"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 2,
   },
   preciousness = {
      frisians = 3,
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 9 },
      },
   }
}
