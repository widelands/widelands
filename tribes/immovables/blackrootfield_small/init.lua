dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "blackrootfield_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Blackroot Field (small)",
   size = "small",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 45000",
			"transform=blackrootfield_medium",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 26, 16 },
      },
   }
}
