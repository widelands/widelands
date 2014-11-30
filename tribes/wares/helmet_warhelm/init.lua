dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "helmet_warhelm",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Warhelm",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"warhelms",
   default_target_quantity = {
		barbarians = 1
	},
   preciousness = {
		barbarians = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Warhelm
		barbarians = _"A warhelm is an enhanced armor for barbarian warriors. It is produced in a helm smithy and used in a training camp – together with food – to train soldiers from health level 1 to level 2."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 12 },
      },
   }
}
