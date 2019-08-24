dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "rope",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Rope"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      amazons = 20
   },
   preciousness = {
      amazons = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 16 },
      },
   }
}
