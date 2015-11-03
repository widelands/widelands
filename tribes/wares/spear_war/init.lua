dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "spear_war",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "War Spear"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 1
	},

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 4, 3 },
      },
   }
}
