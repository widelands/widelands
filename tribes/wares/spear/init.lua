dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "spear",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Spear"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "spears"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Spear
		empire = pgettext("empire_ware", "This spear has an iron spike. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 0 to attack level 1.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 10 },
      },
   }
}
