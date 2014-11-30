dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "buckets",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Buckets",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"buckets",
   default_target_quantity = {
		atlanteans = 2
	},
   preciousness = {
		atlanteans = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Buckets
		atlanteans = _"Big buckets for the fish breeder – produced by the toolsmith."
		},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
