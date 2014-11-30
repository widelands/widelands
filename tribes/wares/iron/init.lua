dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "iron",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Iron",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"iron",
   default_target_quantity = {
		atlanteans = 20,
		barbarians = 20,
		empire = 20
	},
   preciousness = {
		atlanteans = 4,
		barbarians = 4,
		empire = 4
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Iron
		default = _"Iron is extracted from iron ore in the smelting works.",
		-- TRANSLATORS: Helptext for a ware: Iron
		atlanteans = _"It is used in the toolsmithy, armor smithy and weapon smithy.",
		-- TRANSLATORS: Helptext for a ware: Iron
		barbarians = _"It is used to produce weapons and tools in the metal workshop, axfactory, war mill and in the helm smithy.",
		-- TRANSLATORS: Helptext for a ware: Iron
		empire = _"Weapons, armor and tools are made of iron."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 9 },
      },
   }
}
