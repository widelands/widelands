dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "iron_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Iron Ore",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"iron ore",
   default_target_quantity = {
		atlanteans = 15,
		barbarians = 15,
		empire = 15
	},
   preciousness = {
		atlanteans = 4,
		barbarians = 4,
		empire = 4
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Iron Ore
		default = _"Iron ore is mined in iron mines. It is smelted in a smelting works to retrieve the iron."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 4 },
      },
   }
}
