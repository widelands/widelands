dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Helmet"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "helmets"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 1
	},
   preciousness = {
		barbarians = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Helmet
		barbarians = pgettext("barbarians_ware", "A helmet is a basic tool to protect warriors. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 0 to level 1.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 8 },
      },
   }
}
