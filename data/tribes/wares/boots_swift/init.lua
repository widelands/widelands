dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "atlanteans_ware",
   name = "boots_swift",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("atlanteans_ware", "Swift Boots"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      amazons = 1
   },
   preciousness = {
      amazons = 5
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 13 },
      },
   }
}
