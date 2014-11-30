dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "granite",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Granite",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"granite",
   default_target_quantity = {
		atlanteans = 20,
		barbarians = 20,
		empire = 30
	},
   preciousness = {
		atlanteans = 5,
		barbarians = 5,
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Granite
		default = _"Granite is a basic building material.",
		-- TRANSLATORS: Helptext for a ware: Granite
		atlanteans = _"The Atlanteans produce granite blocks in quarries and crystal mines.",
		-- TRANSLATORS: Helptext for a ware: Granite
		barbarians = _"The Barbarians produce granite blocks in quarries and granite mines.",
		-- TRANSLATORS: Helptext for a ware: Granite
		empire = _"The Empire produces granite blocks in quarries and marble mines."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 8 },
      },
   }
}
