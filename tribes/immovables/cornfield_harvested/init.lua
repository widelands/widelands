dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "cornfield_harvested",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"_Cornfield (harvested)",
   attributes = { "field" },
   programs = {
		disappear = {
			"animate=idle 50000",
			"remove=",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 31, 30 },
      },
   }
}
