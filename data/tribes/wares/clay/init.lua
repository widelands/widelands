dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "clay",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Clay"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 30
   },
   preciousness = {
      frisians = 9
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 10 },
      },
   }
}
