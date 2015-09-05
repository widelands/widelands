dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "shield_advanced",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Advanced Shield"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "advanced shields"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 1
	},
   helptext = {
      -- TRANSLATORS: Helptext for a ware: Advanced Shield
		atlanteans = pgettext("atlanteans_ware", "These advanced shields are used by the best soldiers of the Atlanteans. They are produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from Defense level 1 to level 2.")
	},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
