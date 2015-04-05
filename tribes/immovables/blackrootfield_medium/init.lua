dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "blackrootfield_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Blackroot Field (medium)",
   size = "small",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 50000",
			"transform=blackrootfield_ripe",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 26, 30 },
      },
   }
}
