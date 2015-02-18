dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "cornfield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"_Cornfield (ripe)",
   size = "small",
   programs = {
		program = {
			"animate=idle 50000",
			"remove=",
      },
		harvest = {
			"transform=cornfield_harvested",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 31, 41 },
      },
   }
}
