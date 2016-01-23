dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "gold_thread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gold Thread"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 5
   },
   preciousness = {
      atlanteans = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 16 },
      },
   }
}
