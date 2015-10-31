dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "wine",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wine"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 20
   },
   preciousness = {
      empire = 8
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 4, 14 },
      },
   }
}