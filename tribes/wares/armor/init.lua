dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "armor",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Armor"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "suits of armor"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Armor
		empire = pgettext("empire_ware", "Basic armor for Empire soldiers. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 1 to level 2.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 11 },
      },
   }
}
