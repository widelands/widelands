dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fishing_net",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fishing Net"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 2
	},
   preciousness = {
		atlanteans = 0
	},

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 10, 17 },
      },
   }
}
