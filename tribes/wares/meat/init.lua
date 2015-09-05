dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "meat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Meat"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "meat"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20,
		barbarians = 20,
		empire = 20
	},
   preciousness = {
		atlanteans = 2,
		barbarians = 3,
		empire = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Meat
		default = pgettext("default_ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
		-- TRANSLATORS: Helptext for a ware: Meat
		atlanteans = pgettext("atlanteans_ware", "Meat has to be smoked in a smokery before being delivered to mines and training sites (dungeon and labyrinth)."),
		-- TRANSLATORS: Helptext for a ware: Meat
		barbarians = pgettext("barbarians_ware", "Meat is used in the taverns, inns and big inns to prepare rations, snacks and meals for the miners. It is also consumed at the training sites (training camp and battle arena)."),
		-- TRANSLATORS: Helptext for a ware: Meat
		empire = pgettext("empire_ware", "Meat can also be obtained as pork from piggeries. It is used in the inns and taverns to prepare lunch for the miners and is consumed at the training sites (arena, colosseum, training camp).")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 6 },
      },
   }
}
