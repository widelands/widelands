dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "hook_pole",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Hook Pole"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "hook poles"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Hook Pole
		atlanteans = pgettext("atlanteans_ware", "This hook pole is used by the smoker to suspend all the meat and fish from the top of the smokery. It is created by the toolsmith.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 14 },
      },
   }
}
