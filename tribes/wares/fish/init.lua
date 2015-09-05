dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fish"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "fish"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20,
		barbarians = 20,
		empire = 10
	},
   preciousness = {
		atlanteans = 4,
		barbarians = 3,
		empire = 3
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Fish
		atlanteans = pgettext("atlanteans_ware", "Fish is one of the biggest food resources of the Atlanteans. It has to be smoked in a smokery before being delivered to mines, training sites and scouts."),
		-- TRANSLATORS: Helptext for a ware: Fish
		barbarians = pgettext("barbarians_ware", "Besides pitta bread and meat, fish is also a foodstuff for the barbarians. It is used in the taverns, inns and big inns and at the training sites (training camp and battle arena)."),
		-- TRANSLATORS: Helptext for a ware: Fish
		empire = pgettext("empire_ware", "Besides bread and meat, fish is also a foodstuff for the Empire. Fish are used in taverns, inns and training sites (arena, colosseum, training camp).")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 6 },
      },
   }
}
