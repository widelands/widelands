dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "cornmeal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cornmeal"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 15
   },
   preciousness = {
      atlanteans = 7
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 4, 15 },
      },
   }
}