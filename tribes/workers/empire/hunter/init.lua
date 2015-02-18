dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 10, 21 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {9, 32}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {9, 32}, 10)


tribes:new_worker_type {
   name = "empire_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Hunter",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1,
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
