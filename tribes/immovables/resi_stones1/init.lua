dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "new_immovable_type",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = _"Some Stones",
   attributes = { "resi" },
   programs = {
		program = {
			"animate=idle 600000",
			"remove="
      }
   },
   helptext = {
		-- TRANSLATORS: Helptext for a resource: Stones
		atlanteans = _"Precious stones are used in the construction of big buildings. They can be dug up by a crystal mine." .. " " .. _"There are only a few precious stones here.",
		-- TRANSLATORS: Helptext for a resource: Stones
		barbarians = _"Granite is a basic building material and can be dug up by a granite mine." .. " " .. _"There is only a little bit of granite here.",
		-- TRANSLATORS: Helptext for a resource: Stones
		empire = _"Marble is a basic building material and can be dug up by a marble mine. You will also get granite from the mine." .. " " .. _"There is only a little bit of marble here."
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 7, 10 },
      },
   }
}
