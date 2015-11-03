dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "blackroot_flour",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Blackroot Flour"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 15
   },
   preciousness = {
      atlanteans = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 15 },
      },
   }
}
