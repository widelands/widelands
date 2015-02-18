dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "field_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Field (medium)",
   size = "small",
   programs = {
		program = {
			"animate=idle 50000",
			"transform=field_ripe",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 15, 12 },
      },
   }
}
