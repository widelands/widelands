dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "helmet_warhelm",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Warhelm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 1
   },
   preciousness = {
      barbarians = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 12 },
      },
   }
}
