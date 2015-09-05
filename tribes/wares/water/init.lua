dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "water",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Water"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "water"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 25,
		barbarians = 25,
		empire = 25
	},
   preciousness = {
		atlanteans = 7,
		barbarians = 8,
		empire = 7
	},

   helptext = {
		-- TRANSLATORS: Helptext for a ware: Water
		default = pgettext("default_ware", "Water is the essence of life!"),
		-- TRANSLATORS: Helptext for a ware: Water
		atlanteans = pgettext("atlanteans_ware", "Water is used in the bakery and the horse and spider farms."),
		-- TRANSLATORS: Helptext for a ware: Water
		barbarians = pgettext("barbarians_ware", "Water is used in the bakery, the micro brewery and the brewery. The lime kiln and the cattle farm also need to be supplied with water."),
		-- TRANSLATORS: Helptext for a ware: Water
		empire = _"Water is used in the bakery and the brewery. The donkey farm, the sheep farm and the piggery also need to be supplied with water."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 8 },
      },
   }
}
