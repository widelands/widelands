dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "frisians_ware",
   name = "bread_frisians",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("frisians_ware", "Bread"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 20
   },
   preciousness = {
      frisians = 4
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 6 },
      },
   }
}
