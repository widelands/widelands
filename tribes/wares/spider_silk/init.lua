dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "spider_silk",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Spider Silk"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 10
   },
   preciousness = {
      atlanteans = 11
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 9, 16 },
      },
   }
}
