dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "wheat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wheat"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "wheat"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 25,
		empire = 25
	},
   preciousness = {
		barbarians = 12,
		empire = 12
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Wheat
		default = pgettext("default_ware", "Wheat is essential for survival."),
		-- TRANSLATORS: Helptext for a ware: Wheat
		barbarians = pgettext("barbarians_ware", "Wheat is produced by farms and consumed by bakeries, micro breweries and breweries. Cattle farms also need to be supplied with wheat."),
		-- TRANSLATORS: Helptext for a ware: Wheat
		empire = _"Wheat is produced by farms and used by mills and breweries. Donkey farms, sheep farms and piggeries also need to be supplied with wheat."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { -1, 6 },
      },
   }
}
