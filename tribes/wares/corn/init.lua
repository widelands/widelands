dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "corn",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Corn"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 25
   },
   preciousness = {
      atlanteans = 12
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 6, 11 },
      },
   }
}
