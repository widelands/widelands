dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "empire_shipconstruction",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Ship Under Construction"),
   size = "small",
   programs = {
		program = {
			"construction=idle 5000 210000",
			"transform=bob tribe:empire_ship",
      }
   },
   buildcost = {
      planks = 10,
      log = 2,
      cloth = 4
   },

   animations = {
      idle = {
         template = "build_??",
         directory = dirname,
         hotspot = { 115, 78 },
         fps = 1
      },
   }
}
