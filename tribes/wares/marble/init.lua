dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "marble",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Marble"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 30
   },
   preciousness = {
      empire = 3
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 10, 18 },
      },
   }
}
