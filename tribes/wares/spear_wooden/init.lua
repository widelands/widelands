dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "spear_wooden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wooden Spear"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 30
   },
   preciousness = {
      empire = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 11 },
      },
   }
}
