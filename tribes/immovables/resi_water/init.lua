dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = _"Water Vein",
   attributes = { "resi" },
   programs = {
		program = {
			"animate=idle 600000",
			"remove="
      }
   },
   helptext = {
		-- TRANSLATORS: Helptext for a resource: Water
		default = _"There is water in the ground here that can be pulled up by a well."
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 7, 10 },
      },
   }
}
