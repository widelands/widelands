dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "beer",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Beer",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"beer",
   default_target_quantity = {
		barbarians = 15,
		empire = 15
	},
   preciousness = {
		barbarians = 2,
		empire = 5
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Beer
		barbarians = _"Beer is produced in micro breweries used in inns and big inns to produce snacks.",
		-- TRANSLATORS: Helptext for a ware: Beer
		empire = _"This beer is produced in a brewery out of wheat and water. It is consumed by miners in coal and iron ore mines."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 9 },
      },
   }
}
