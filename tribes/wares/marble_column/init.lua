dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "marble_column",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Marble Column"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 10
   },
   preciousness = {
      empire = 5
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 3, 9 },
      },
   }
}
