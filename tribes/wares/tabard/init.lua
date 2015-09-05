dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "tabard",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Tabard"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "tabards"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 30
	},
   preciousness = {
		atlanteans = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Tabard
		atlanteans = pgettext("atlanteans_ware", "A tabard and a light trident are the basic equipment for young soldiers. Tabards are produced in the weaving mill.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 10 },
      },
   }
}
