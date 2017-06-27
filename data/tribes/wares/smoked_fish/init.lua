dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "smoked_fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Smoked Fish"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 30,
      frisians = 30
   },
   preciousness = {
      atlanteans = 3,
      frisians = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 16 },
      },
   }
}
