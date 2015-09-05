dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "meal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Meal"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "meal"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 10,
		empire = 20
	},
   preciousness = {
		barbarians = 5,
		empire = 4
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Meal
		barbarians = pgettext("barbarians_ware", "A meal is made out of pitta bread, stout and fish/meat in a big inn. This substantial food is exactly what workers in a deeper mine need."),
		-- TRANSLATORS: Helptext for a ware: Meal
		empire = pgettext("empire_ware", "A real meal is made in inns out of bread and fish/meat. It satisfies the needs of miners in deep mines.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 6 },
      },
   }
}
