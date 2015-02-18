dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "resi_gold2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = _"Main Gold Vein",
   attributes = { "resi" },
   programs = {
		program = {
			"animate=idle 600000",
			"remove="
      }
   },
   helptext = {
		-- TRANSLATORS: Helptext for a resource: Gold
		default = _"Gold veins contain gold ore that can be dug up by gold mines." .. " " .. _"There is a lot of gold here."
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 7, 10 },
      },
   }
}
