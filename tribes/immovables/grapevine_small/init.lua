dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "grapevine_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Grapevine (small)",
   size = "medium",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 28000",
			"transform=grapevine_medium",
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
