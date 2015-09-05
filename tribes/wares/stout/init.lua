dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "stout",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Stout"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "stout"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 15
	},
   preciousness = {
		barbarians = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Stout
		barbarians = pgettext("barbarians_ware", "Only this beer is acceptable for the warriors in a battle arena. Some say that the whole power of the barbarians lies in this ale. It helps to train the soldiers’ evade level from 0 to 1 to 2. Stout is also used in big inns to prepare meals.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 8 },
      },
   }
}
