dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "blackroot",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Blackroot"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "blackroots"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20
	},
   preciousness = {
		atlanteans = 10
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Blackroot
		atlanteans = pgettext("atlanteans_ware", "Blackroots are a special kind of root produced at blackroot farms and processed in mills. The Atlanteans like their strong taste and use their flour for making bread.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 15 },
      },
   }
}
