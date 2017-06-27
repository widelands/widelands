dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "gold",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gold"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 20,
      barbarians = 20,
      frisians = 20,
      empire = 20
   },
   preciousness = {
      atlanteans = 2,
      barbarians = 2,
      frisians = 2,
      empire = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 10 },
      },
   }
}
