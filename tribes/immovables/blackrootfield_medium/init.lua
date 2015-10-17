dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "blackrootfield_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Blackroot Field (medium)"),
   size = "small",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 50000",
			"transform=blackrootfield_ripe",
      }
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 26, 30 },
      },
   }
}
