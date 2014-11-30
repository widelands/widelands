dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "milking_tongs",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Milking Tongs",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"milking tongs",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Milking Tongs
		atlanteans = _"Milking tongs are used by the spider breeder to milk the spiders. They are produced by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 9 },
      },
   }
}
