dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "empire_ware",
   name = "empire_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("empire_ware", "Bread"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 20
   },
   preciousness = {
      empire = 7
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 6 },
      },
   }
}
