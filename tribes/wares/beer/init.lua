dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "beer",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Beer"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "beer"),
   icon = dirname .. "menu.png",
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
		barbarians = pgettext("barbarians_ware", "Beer is produced in micro breweries and used in inns and big inns to produce snacks."),
		-- TRANSLATORS: Helptext for a ware: Beer
		empire = pgettext("empire_ware", "This beer is produced in a brewery out of wheat and water. It is consumed by miners in coal and iron ore mines.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 9 },
      },
   }
}
