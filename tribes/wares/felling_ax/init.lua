dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "felling_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Felling Ax"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 5,
		empire = 3
	},
   preciousness = {
		barbarians = 3,
		empire = 1
	},

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 6, 7 },
      },
   }
}
