dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 13, 26 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {6, 26}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {6, 26}, 10)


tribes:new_worker_type {
   name = "barbarians_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Hunter",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1,
		hunting_spear = 1
	},

	programs = {
		hunt = {
			"findobject type:bob radius:13 attrib:eatable",
			"walk object",
			"animation idle 1000",
			"object remove",
			"createware meat",
			"return"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Hunter
   helptext = _"The hunter brings fresh, raw meat to the colonists.",
   animations = animations,
}
