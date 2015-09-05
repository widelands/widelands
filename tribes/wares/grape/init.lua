dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "grape",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Grape"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "grapes"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 20
	},
   preciousness = {
		empire = 10
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Grape
		empire = pgettext("empire_ware", "These grapes are the base for a tasty wine. They are harvested in a vineyard and processed in a winery.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 15 },
      },
   }
}
