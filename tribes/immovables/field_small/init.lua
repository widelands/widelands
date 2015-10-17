dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "field_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Field (small)"),
   size = "small",
   attributes = { "field" },
   programs = {
		program = {
			"animate=idle 45000",
			"transform=field_medium",
      }
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 15, 9 },
      },
   }
}
