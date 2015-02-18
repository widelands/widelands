dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 4, 23 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {8, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {8, 23}, 10)

tribes:new_worker_type {
   name = "barbarians_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Trainer",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1,
		ax = 1
	},

	-- TRANSLATORS: Helptext for a worker: Trainer
   helptext = _"Trains the soldiers.",
   animations = animations,
}
