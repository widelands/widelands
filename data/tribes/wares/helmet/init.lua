dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Helmet"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 1,
      frisians = 2
   },
   preciousness = {
      barbarians = 1,
      frisians = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 8 },
      },
   }
}
