dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 6, 28 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 24}, 15)
add_worker_animations(animations, "walkload", dirname, "walk", {11, 24}, 15)


tribes:new_worker_type {
   name = "barbarians_blacksmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Blacksmith",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1,
		hammer = 1
	},

	experience = 24,
	becomes = "barbarians_blacksmith_master",

	-- TRANSLATORS: Helptext for a worker: Blacksmith
   helptext = _"Produces weapons for soldiers and tools for workers.",
   animations = animations,
}
