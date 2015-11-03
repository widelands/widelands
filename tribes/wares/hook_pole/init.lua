dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "hook_pole",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Hook Pole"),
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
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 14 },
      },
   }
}
