dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "spidercloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Spidercloth"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 20
   },
   preciousness = {
      atlanteans = 7
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 6, 9 },
      },
   }
}
