dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "reed_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Reed (medium)",
   size = "small",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 40000",
			"transform=reed_ripe",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 13, 14 },
      },
   }
}
