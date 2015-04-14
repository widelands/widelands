dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "hammer",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Hammer",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"hammers",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 2,
		barbarians = 2,
		empire = 2
	},
   preciousness = {
		atlanteans = 1,
		barbarians = 1,
		empire = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Hammer
		default = _"The hammer is an essential tool.",
		-- TRANSLATORS: Helptext for a ware: Hammer
		atlanteans = _"Geologists, builders, weaponsmiths and armorsmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the toolsmith.",
		-- TRANSLATORS: Helptext for a ware: Hammer
		barbarians = _"Geologists, builders, blacksmiths and helmsmiths all need a hammer. Make sure you’ve always got some in reserve! They are one of the basic tools produced at the metal workshop (but cease to be produced by the building if it is enhanced to an axfactory and war mill).",
		-- TRANSLATORS: Helptext for a ware: Hammer
		empire = _"Geologists, builders, weaponsmiths and armorsmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 4 },
      },
   }
}
