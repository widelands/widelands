dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "smoked_fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Smoked Fish"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "smoked fish"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 30
	},
   preciousness = {
		atlanteans = 3
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Smoked Fish
		atlanteans = pgettext("atlanteans_ware", "As no Atlantean likes raw fish, smoking it in a smokery is the most common way to make it edible.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
