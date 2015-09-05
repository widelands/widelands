dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "spear_heavy",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Heavy Spear"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "heavy spears"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Heavy Spear
		empire = pgettext("empire_ware", "This is a strong spear with a steel-tip and a little blade. It is produced in the weapon smithy and used in the training camp – together with food – train soldiers from attack level 2 to level 3.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 11 },
      },
   }
}
