dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "buckets",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Buckets"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "buckets"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 2
	},
   preciousness = {
		atlanteans = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Buckets
		atlanteans = pgettext("atlanteans_ware", "Big buckets for the fish breeder – produced by the toolsmith.")
		},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
