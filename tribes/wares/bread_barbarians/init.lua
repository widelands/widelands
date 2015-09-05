dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarians_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Pitta Bread"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "pitta bread"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 20
	},
   preciousness = {
		barbarians = 4
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Pitta Bread
		barbarians = pgettext("barbarians_ware", "The barbarian bakers are best in making this flat and tasty pitta bread. It is made out of wheat and water following a secret recipe. Pitta bread is used in the taverns, inns and big inns to prepare rations, snacks and meals. It is also consumed at training sites (training camp and battle arena).")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 6 },
      },
   }
}
