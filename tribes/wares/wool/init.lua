dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "wool",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wool"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 10
   },
   preciousness = {
      empire = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 8, 18 },
      },
   }
}
