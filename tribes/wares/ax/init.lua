dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Ax",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"axes",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 30
	},
   preciousness = {
		barbarians = 3
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Ax
		barbarians = _"The ax is the basic weapon of the barbarians. All young soldiers are equipped with it."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}
