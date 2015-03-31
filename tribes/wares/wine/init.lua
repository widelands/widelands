dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "wine",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Wine",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"wine",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 20
   },
   preciousness = {
		empire = 8
   },
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Wine
		empire = _"This tasty wine is drunk by the miners working the marble and gold mines. It is produced in a winery."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 14 },
      },
   }
}
