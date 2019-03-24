dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "coal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Coal"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 20,
      barbarians = 20,
      frisians = 20,
      empire = 20
   },
   preciousness = {
      atlanteans = 1,
      barbarians = 1,
      frisians = 6,
      empire = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 6 },
      },
   }
}
