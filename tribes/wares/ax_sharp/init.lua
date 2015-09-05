dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "ax_sharp",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Sharp Ax"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "sharp axes"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 1
	},
   preciousness = {
		barbarians = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Sharp Ax
		barbarians = pgettext("barbarians_ware", "Young warriors are proud to learn to fight with this powerful weapon. It is heavier and sharper than the ordinary ax. It is produced in axfactories and war mills. In training camps, it is used – together with food – to train soldiers from attack level 0 to level 1.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}
