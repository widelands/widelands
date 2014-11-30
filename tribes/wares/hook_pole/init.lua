dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "hook_pole",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Hook Pole",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"hook poles",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Hook Pole
		atlanteans = _"This hook pole is used by the smoker to suspend all the meat and fish from the top of the smokery. It is created by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 14 },
      },
   }
}
