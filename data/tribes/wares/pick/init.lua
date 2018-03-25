dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "pick",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Pick"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 3,
      barbarians = 2,
      frisians = 3,
      empire = 2
   },
   preciousness = {
      atlanteans = 1,
      barbarians = 1,
      frisians = 0,
      empire = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 4 },
      },
   }
}
