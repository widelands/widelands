dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "saw",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Saw"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 2,
      empire = 1
   },
   preciousness = {
      atlanteans = 0,
      empire = 0
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 7, 5 },
      },
   }
}
