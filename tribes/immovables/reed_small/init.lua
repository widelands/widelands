dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "reed_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Reed (small)",
   size = "small",
   programs = {
		program = {
			"animate=idle 28000",
			"transform=reed_medium",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 13, 12 },
      },
   }
}
