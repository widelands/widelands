dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "diamond",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Diamond"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "diamonds"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 5
	},
   preciousness = {
		atlanteans = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Diamond
		atlanteans = pgettext("atlanteans_ware", "These wonderful diamonds are used to build some exclusive buildings. They are mined in a crystal mine.")
	},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 7 },
      },
   }
}
