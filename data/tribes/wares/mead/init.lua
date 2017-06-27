dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "mead",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Mead"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 15,
   },
   preciousness = {
      frisians = 4,
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 9 },
      },
   }
}
