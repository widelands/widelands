dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "fishing_net",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fishing Net",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"fishing nets",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 2
	},
   preciousness = {
		atlanteans = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Fishing Net
		atlanteans = _"The fishing net is used by the fisher and produced by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
