dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "cornmeal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Cornmeal",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"cornmeal",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 15
	},
   preciousness = {
		atlanteans = 7
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Cornmeal
		atlanteans = _"Cornmeal is produced in a mill out of corn and is one of three parts of the Atlantean bread produced in bakeries."
	},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 15 },
      },
   }
}
