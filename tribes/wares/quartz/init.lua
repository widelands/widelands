dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "quartz",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Quartz",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"quartz",
   default_target_quantity = {
		atlanteans = 5
	},
   preciousness = {
		atlanteans = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Quartz
		atlanteans = _"These transparent quartz gems are used to build some exclusive buildings. They are produced in a crystal mine."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 13 },
      },
   }
}
