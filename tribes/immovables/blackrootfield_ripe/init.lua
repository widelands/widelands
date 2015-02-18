dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "blackrootfield_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Blackroot Field (ripe)",
   size = "small",
   attributes = { "ripe_blackroot" },
   programs = {
		program = {
			"animate=idle 50000",
			"remove=",
      },
		harvest = {
			"transform=blackrootfield_harvested",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 28, 45 },
      },
   }
}
