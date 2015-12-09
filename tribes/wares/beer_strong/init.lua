dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "beer_strong",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Strong Beer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 15
   },
   preciousness = {
      barbarians = 2
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 5, 8 },
      },
   }
}
