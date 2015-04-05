dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "grapevine_ripe",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Grapevine (ripe)",
   size = "medium",
   attributes = { "ripe_grapes", "field" },
   programs = {
		program = {
			"animate=idle 500000",
			"remove=",
      },
		harvest = {
			"remove=",
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
