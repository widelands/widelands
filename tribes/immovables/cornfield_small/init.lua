dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "cornfield_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"_Cornfield (small)",
   size = "small",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 45000",
			"transform=cornfield_medium",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 35, 34 },
      },
   }
}
