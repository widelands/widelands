dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "saw",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Saw"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "saws"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 2,
		empire = 1
	},
   preciousness = {
		atlanteans = 0,
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Saw
		atlanteans = pgettext("atlanteans_ware", "The saw is needed by the woodcutter and the toolsmith. It is produced by the toolsmith."),
		-- TRANSLATORS: Helptext for a ware: Saw
		empire = pgettext("empire_ware", "This saw is needed by the sawyer. It is produced by the toolsmith.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 5 },
      },
   }
}
