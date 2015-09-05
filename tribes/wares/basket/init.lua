dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "basket",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Basket"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "baskets"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Basket
		empire = pgettext("empire_ware", "This basket is needed by the vinefarmer for harvesting the grapes. It is produced by the toolsmith.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 11 },
      },
   }
}
