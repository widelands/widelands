dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "shield_steel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Steel Shield",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"steel shields",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Steel Shield
		atlanteans = _"This steel shield is produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from defense level 0 to level 1."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
