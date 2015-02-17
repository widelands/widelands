dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "trident_light",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Light Trident",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"light tridents",
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 30
	},
   preciousness = {
		atlanteans = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Light Trident
		atlanteans = _"This is the basic weapon of the Atlantean soldiers. Together with a tabard, it makes up the equipment of young soldiers. Light tridents are produced in the weapon smithy as are all other tridents."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
