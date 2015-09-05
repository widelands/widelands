dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "marble_column",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Marble Column"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "marble columns"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 10
	},
   preciousness = {
		empire = 5
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Marble Column
		empire = pgettext("empire_ware", "Marble columns represent the high culture of the Empire, so they are needed for nearly every larger building. They are produced out of marble at a stonemason’s house.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 9 },
      },
   }
}
