dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "coal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Coal",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"coal",
   default_target_quantity = {
		atlanteans = 20,
		barbarians = 20,
		empire = 20
	},
   preciousness = {
		atlanteans = 1,
		barbarians = 1,
		empire = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Coal
		default = _"Coal is mined in coal mines or produced out of logs by a charcoal kiln.",
		-- TRANSLATORS: Helptext for a ware: Coal
		atlanteans = _"The Atlantean fires in smelting works, armor smithies and weapon smithies are fed with coal.",
		-- TRANSLATORS: Helptext for a ware: Coal
		barbarians = _"The fires of the barbarians are usually fed with coal. Consumers are several buildings: lime kiln, smelting works, axfactory, war mill, and helm smithy.",
		-- TRANSLATORS: Helptext for a ware: Coal
		empire = _"The fires of the Empire smelting works, armor smithies and weapon smithies are usually fed with coal."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 6 },
      },
   }
}
