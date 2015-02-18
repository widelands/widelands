dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "resi_coal1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = _"Coal Vein",
   attributes = { "resi" },
   programs = {
		program = {
			"animate=idle 600000",
			"remove="
      }
   },
   helptext = {
		-- TRANSLATORS: Helptext for a resource: Coal
		default = _"Coal veins contain coal that can be dug up by coal mines." .. " " .. _"There is only a little bit of coal here."
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 7, 10 },
      },
   }
}
