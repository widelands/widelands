dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "planks",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Planks"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "planks"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 40,
		empire = 40
	},
   preciousness = {
		atlanteans = 7,
		empire = 7
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Planks
		default = pgettext("default_ware", "Planks are an important building material. They are produced out of logs by the sawmill."),
		-- TRANSLATORS: Helptext for a ware: Planks
		atlanteans = pgettext("atlanteans_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships."),
		-- TRANSLATORS: Helptext for a ware: Planks
		empire = pgettext("empire_ware", "They are also used by the weapon smithy.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 12, 17 },
      },
   }
}
