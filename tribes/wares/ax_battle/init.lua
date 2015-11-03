dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "ax_battle",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Battle Ax"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 1
	},
   preciousness = {
		barbarians = 1
	},

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 7, 7 },
      },
   }
}
