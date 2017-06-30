dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "cloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cloth"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 10,
      empire = 15,
      frisians = 10
   },
   preciousness = {
      barbarians = 0,
      empire = 1,
      frisians = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 9 },
      },
   }
}
