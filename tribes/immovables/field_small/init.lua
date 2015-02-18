dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "field_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Field (small)",
   size = "small",
   programs = {
		program = {
			"animate=idle 45000",
			"transform=field_medium",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 15, 9 },
      },
   }
}
