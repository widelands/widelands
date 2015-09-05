dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "kitchen_tools",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Kitchen Tools"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "kitchen tools"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 1,
		empire = 1
	},
   preciousness = {
		barbarians = 0,
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Kitchen Tools
		barbarians = pgettext("barbarians_ware", "How can one create a meal, snack or ration, if there are no kitchen tools? Be sure to have a metal workshop to produce this basic tool (but it ceases to be produced by the building if it is enhanced to an axfactory and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Kitchen Tools
		empire = pgettext("empire_ware", "How can one create a ration or meal if there are no kitchen tools? They are produced in a toolsmithy and used in taverns and inns.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 8 },
      },
   }
}
