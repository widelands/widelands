dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "fire_tongs",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fire Tongs",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"fire tongs",
   default_target_quantity = {
		atlanteans = 1,
		barbarians = 1,
		empire = 1
	},
   preciousness = {
		atlanteans = 0,
		barbarians = 0,
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		default = _"Fire tongs are the tools of the smelter, who works in the smelting works.",
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		atlanteans = _"They are produced by the toolsmith.",
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		barbarians = _"Produced by the metal works (but they cease to be produced by the building if it is enhanced to an axfactory and war mill).",
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		empire = _"They are produced by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 5 },
      },
   }
}
