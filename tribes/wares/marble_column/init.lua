dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "marble_column",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Marble Column",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"marble columns",
   default_target_quantity = {
		empire = 10
	},
   preciousness = {
		empire = 5
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Marble Column
		empire = _"Marble columns represent the high culture of the Empire, so they are needed for nearly every larger building. They are produced out of marble at a stonemason’s house."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 9 },
      },
   }
}
