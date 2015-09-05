dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "gold_thread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gold Thread"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "gold thread"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 5
	},
   preciousness = {
		atlanteans = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Gold Thread
		atlanteans = pgettext("atlanteans_ware", "This thread, made of gold by the gold spinning mill, is used for weaving the exclusive golden tabard in the weaving mill.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
