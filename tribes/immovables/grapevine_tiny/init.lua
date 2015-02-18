dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "grapevine_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Grapevine (tiny)",
   size = "medium",
   programs = {
		program = {
			"animate=idle 22000",
			"transform=grapevine_small",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 15, 18 },
      },
   }
}
