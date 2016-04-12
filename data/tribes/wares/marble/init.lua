dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "marble",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Marble"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      empire = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 18 },
      },
   }
}
