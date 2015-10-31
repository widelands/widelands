dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "hunting_spear",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Hunting Spear"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 1,
      empire = 1
   },
   preciousness = {
      barbarians = 0,
      empire = 0
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 6, 5 },
      },
   }
}