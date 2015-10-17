dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cornfield_medium",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Cornfield (medium)"),
   size = "small",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 50000",
			"transform=cornfield_ripe",
      }
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 31, 35 },
      },
   }
}
