dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "ax_battle",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Battle Ax"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "battle axes"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 1
	},
   preciousness = {
		barbarians = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Battle Ax
		barbarians = pgettext("barbarians_ware", "This is a dangerous weapon the barbarians are able to produce. It is produced in the ax factory. Only trained warriors are able to wield such a weapon. It is used – together with food – in the training camp to train soldiers from attack level 3 to 4.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}
