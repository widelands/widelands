dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "marble",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Marble",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"marble",
   default_target_quantity = {
		empire = 30
	},
   preciousness = {
		empire = 3
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Marble
		empire = _"Marble is the beautiful stone which is cut out of the mountains or produced in a quarry. Marble is used as building material and gets chiselled into marble columns in the stonemason’s house."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 18 },
      },
   }
}
