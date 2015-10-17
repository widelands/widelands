dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "reed_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Reed (small)"),
   size = "small",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 28000",
			"transform=reed_medium",
      }
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 13, 12 },
      },
   }
}
