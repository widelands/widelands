dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "brick",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Brick"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 40
   },
   preciousness = {
      frisians = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 9 },
      },
   }
}
