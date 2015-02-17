dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "planks",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Planks",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"planks",
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
		default = _"Planks are an important building material. They are produced out of logs by the sawmill.",
		-- TRANSLATORS: Helptext for a ware: Planks
		atlanteans = _"The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.",
		-- TRANSLATORS: Helptext for a ware: Planks
		empire = _"They are also used by the weapon smithy."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 12, 17 },
      },
   }
}
