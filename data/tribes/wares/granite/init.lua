dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "granite",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Granite"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 20,
      barbarians = 20,
      frisians = 30,
      empire = 30
   },
   preciousness = {
      atlanteans = 5,
      barbarians = 5,
      frisians = 3,
      empire = 0
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 8 },
      },
   }
}
