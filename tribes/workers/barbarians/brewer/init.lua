dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 7, 22 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {23, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {11, 24}, 10)


tribes:new_worker_type {
   name = "barbarians_brewer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Brewer",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1
	},

	experience = 19,
	becomes = "barbarians_brewer_master",

	-- TRANSLATORS: Helptext for a worker: Brewer
   helptext = _"Produces the finest ales to keep warriors strong and happy in training.",
   animations = animations,
}
