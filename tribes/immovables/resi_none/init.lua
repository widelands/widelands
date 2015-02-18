dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = _"No Resources",
   attributes = { "resi" },
   programs = {
		program = {
			"animate=idle 600000",
			"remove="
      }
   },
   helptext = {
		-- TRANSLATORS: Helptext for a resource: No resources
		default = _"There are no resources in the ground here."
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 7, 10 },
      },
   }
}
