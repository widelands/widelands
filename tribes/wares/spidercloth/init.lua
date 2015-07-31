dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "spidercloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Spidercloth",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"spidercloth",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20
	},
   preciousness = {
		atlanteans = 7
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Spidercloth
		atlanteans = _"Spidercloth is made out of spideryarn in a weaving mill. It is used in the toolsmithy and the shipyard. Also some higher developed buildings need spidercloth for their construction."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 9 },
      },
   }
}
