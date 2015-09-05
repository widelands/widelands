dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "smoked_meat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Smoked Meat"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "smoked meat"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20
	},
   preciousness = {
		atlanteans = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Smoked Meat
		atlanteans = pgettext("atlanteans_ware", "Smoked meat is made out of meat in a smokery. It is delivered to the mines and training sites (labyrinth and dungeon) where the miners and soldiers prepare a nutritious lunch for themselves.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
