dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "shovel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Shovel"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 2,
      barbarians = 1,
      empire = 1,
      frisians = 3,
   },
   preciousness = {
      atlanteans = 0,
      barbarians = 0,
      frisians = 0,
      empire = 0
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 5 },
      },
   }
}
