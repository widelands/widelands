dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "atlanteans_ware",
   name = "atlanteans_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("atlanteans_ware", "Bread"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 20
   },
   preciousness = {
      atlanteans = 5
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 9, 13 },
      },
   }
}