dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "resi_iron2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = _"Main Iron Vein",
   attributes = { "resi" },
   programs = {
		program = {
			"animate=idle 600000",
			"remove="
      }
   },
   helptext = {
		-- TRANSLATORS: Helptext for a resource: Iron
		default = _"Iron veins contain iron ore that can be dug up by iron mines." .. " " .. _"There is a lot of iron here."
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 7, 10 },
      },
   }
}
