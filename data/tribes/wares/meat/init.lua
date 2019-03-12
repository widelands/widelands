dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "meat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Meat"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 20
   },
   preciousness = {
      atlanteans = 2,
      barbarians = 3,
      frisians = 2,
      empire = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 6 },
      },
   }
}
