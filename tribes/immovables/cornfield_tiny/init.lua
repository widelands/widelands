dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "cornfield_tiny",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"_Cornfield (tiny)",
   size = "small",
   programs = {
		program = {
			"animate=idle 30000",
			"transform=cornfield_small",
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
