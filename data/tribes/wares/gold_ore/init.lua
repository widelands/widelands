dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "gold_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gold Ore"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 15,
      barbarians = 15,
      frisians = 15,
      empire = 15
   },
   preciousness = {
      atlanteans = 2,
      barbarians = 2,
      frisians = 3,
      empire = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 4 },
      },
   }
}
