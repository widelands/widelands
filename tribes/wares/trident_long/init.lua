dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "trident_long",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Long Trident"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "long tridents"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Long Trident
		atlanteans = pgettext("atlanteans_ware", "The long trident is the first trident in the training of soldiers. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 0 to level 1.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
