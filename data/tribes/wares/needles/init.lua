dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "needles",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Needles"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 1,
   },
   preciousness = {
      frisians = 0,
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 4 },
      },
   }
}
