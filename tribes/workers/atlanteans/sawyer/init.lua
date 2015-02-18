dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 5, 31 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {16, 31}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {16, 31}, 10)


tribes:new_worker_type {
   name = "atlanteans_sawyer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Sawyer",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1,
		saw = 1
	},

	-- TRANSLATORS: Helptext for a worker: Sawyer
   helptext = _"Saws logs to produce planks.",
   animations = animations,
}
