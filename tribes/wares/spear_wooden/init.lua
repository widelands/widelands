dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "spear_wooden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wooden Spear"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 30
   },
   preciousness = {
      empire = 1
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 4, 11 },
      },
   }
}