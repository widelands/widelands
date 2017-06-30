dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "water",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Water"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {},
   preciousness = {
      atlanteans = 7,
      barbarians = 8,
      frisians = 2,
      empire = 7,
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 8 },
      },
   }
}
