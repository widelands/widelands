dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "hunting_spear",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Hunting Spear"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 1,
      frisians = 1,
      empire = 1
   },
   preciousness = {
      barbarians = 0,
      frisians = 0,
      empire = 0
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 5 },
      },
   }
}
