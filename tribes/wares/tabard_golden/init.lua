dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "tabard_golden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Golden Tabard"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "golden tabards"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Golden Tabard
		atlanteans = pgettext("atlanteans_ware", "Golden tabards are produced in Atlantean weaving mills out of gold thread. They are used in the labyrinth – together with food – to train soldiers from health level 0 to level 1.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 10 },
      },
   }
}
