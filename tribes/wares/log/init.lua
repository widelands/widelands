dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "log",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Log"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 40,
      barbarians = 40,
      empire = 40
   },
   preciousness = {
      atlanteans = 14,
      barbarians = 14,
      empire = 14
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 7 },
      },
   }
}
