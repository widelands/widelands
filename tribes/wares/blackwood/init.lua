dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "blackwood",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Blackwood"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 40
   },
   preciousness = {
      barbarians = 10
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 10, 10 },
      },
   }
}
