dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "milking_tongs",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Milking Tongs"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 1
   },
   preciousness = {
      atlanteans = 0
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 6, 9 },
      },
   }
}
