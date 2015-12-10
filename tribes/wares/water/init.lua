dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "water",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Water"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 25,
      barbarians = 25,
      empire = 25
   },
   preciousness = {
      atlanteans = 7,
      barbarians = 8,
      empire = 7
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 4, 8 },
      },
   }
}
