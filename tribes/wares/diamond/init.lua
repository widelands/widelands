dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "diamond",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Diamond",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"diamonds",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 5
	},
   preciousness = {
		atlanteans = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Diamond
		atlanteans = _"These wonderful diamonds are used to build some exclusive buildings. They are mined in a crystal mine."
	},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 7 },
      },
   }
}
