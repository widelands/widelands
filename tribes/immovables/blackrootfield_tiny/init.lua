dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "blackrootfield_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Blackroot Field (tiny)",
   size = "small",
   programs = {
		program = {
			"animate=idle 30000",
			"transform=blackrootfield_small",
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
