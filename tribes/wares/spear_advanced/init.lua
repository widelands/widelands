dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "spear_advanced",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Advanced Spear",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"advanced spears",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 1
	},
   -- TRANSLATORS: Helptext for a ware: Advanced Spear
   helptext = {
		empire = _"This is an advanced spear with a steel tip. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 1 to level 2."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 10 },
      },
   }
}
