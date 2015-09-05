dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "blackroot_flour",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Blackroot Flour"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "blackroot flour"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 15
	},
   preciousness = {
		atlanteans = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Blackroot Flour
		atlanteans = pgettext("atlanteans_ware", "Blackroot Flour is produced in mills out of blackroots. It is used in bakeries to make a tasty bread.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 15 },
      },
   }
}
