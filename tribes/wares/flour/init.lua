dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "flour",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Flour"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 20
	},
   preciousness = {
		empire = 9
	},

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 5, 15 },
      },
   }
}
