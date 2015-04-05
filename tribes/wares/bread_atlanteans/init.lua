dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlanteans_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Bread",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"bread",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20
	},
   preciousness = {
		atlanteans = 5
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Bread
		atlanteans = _"This tasty bread is made in bakeries out of cornmeal, blackroot flour and water. It is appreciated as basic food by miners, scouts and soldiers in training sites (labyrinth and dungeon)."
		},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 13 },
      },
   }
}
