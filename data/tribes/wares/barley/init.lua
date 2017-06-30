dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "barley",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Barley"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      frisians = 5
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 15 },
      },
   }
}
