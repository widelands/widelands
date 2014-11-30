dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "armor_gilded",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Gilded Armor",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"suits of gilded armor",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 1
	},
   helptext =  {
		-- TRANSLATORS: Helptext for a ware: Gilded Armor
		empire = _"The gilded armor is the strongest armor an Empire soldier can have. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 3 to level 4."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 11 },
      },
   }
}
