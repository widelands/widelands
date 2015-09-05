dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "armor_chain",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Chain Armor"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "suits of chain armor"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Chain Armor
		empire = pgettext("empire_ware", "The chain armor is a medium armor for Empire soldiers. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 2 to level 3.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 11 },
      },
   }
}
