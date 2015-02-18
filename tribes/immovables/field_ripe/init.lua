dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "field_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Field (ripe)",
   size = "small",
   programs = {
		program = {
			"animate=idle 500000",
			"remove=",
		},
		harvest = {
			"transform=field_harvested"
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
