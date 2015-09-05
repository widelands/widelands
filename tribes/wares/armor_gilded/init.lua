dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "armor_gilded",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gilded Armor"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "suits of gilded armor"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 1
	},
   helptext =  {
		-- TRANSLATORS: Helptext for a ware: Gilded Armor
		empire = pgettext("empire_ware", "The gilded armor is the strongest armor an Empire soldier can have. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 3 to level 4.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 11 },
      },
   }
}
