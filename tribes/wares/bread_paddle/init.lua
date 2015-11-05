dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "bread_paddle",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Bread Paddle"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 1,
      barbarians = 1,
      empire = 1
   },
   preciousness = {
      atlanteans = 0,
      barbarians = 0,
      empire = 0
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 6, 6 },
      },
   }
}
