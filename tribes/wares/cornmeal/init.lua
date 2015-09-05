dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "cornmeal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cornmeal"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "cornmeal"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 15
	},
   preciousness = {
		atlanteans = 7
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Cornmeal
		atlanteans = pgettext("atlanteans_ware", "Cornmeal is produced in a mill out of corn and is one of three parts of the Atlantean bread produced in bakeries.")
	},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 15 },
      },
   }
}
