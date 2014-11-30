dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "wool",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Wool",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"wool",
   default_target_quantity = {
		empire = 10
	},
   preciousness = {
		empire = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Wool
		empire = _"Wool is the hair of sheep. Weaving mills use it to make cloth."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 18 },
      },
   }
}
