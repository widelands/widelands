dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "resi_granite1", -- This is named "granite" for compatibility with World.
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Some Stones"),
   attributes = { "resi" },
   programs = {
		program = {
			"animate=idle 600000",
			"remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 7, 10 },
      },
   }
}
