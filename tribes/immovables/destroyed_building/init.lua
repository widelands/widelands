dirname = path.dirname(__file__)

tribes:new_immovable_type{
   name = "destroyed_building",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Destroyed building",
   size = "big",
   programs = {
		program = {
			"animate=burn 30000",
			"transform=ashes",
      }
   },
   helptext = {
		default = ""
   },
   animations = {
      burn = {
         pictures = path.list_directory(dirname, "burn_\\d+.png"),
         hotspot = { 40, 57 },
         fps = 10,
      },
   }
}
