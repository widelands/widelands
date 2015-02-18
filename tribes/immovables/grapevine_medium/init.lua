dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "grapevine_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Grapevine (medium)",
   size = "medium",
   programs = {
		program = {
			"animate=idle 40000",
			"transform=grapevine_ripe",
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
