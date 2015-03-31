dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "shipconstruction_atlanteans",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = _"Ship Under Construction",
   size = "small",
   programs = {
		program = {
			"construction=idle 5000 210000",
			"transform=bob tribe:ship_atlanteans",
      }
   },
   buildcost = {
      planks = 10,
      log = 2,
      spidercloth = 4
   },
   helptext = {
		default = ""
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "build_\\d+.png"),
         hotspot = { 118, 94 },
         fps = 1
      },
   }
}
