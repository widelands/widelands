dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 6, 23 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {9, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {9, 23}, 10)


tribes:new_worker_type {
   name = "empire_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Trainer",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1,
		armor_helmet = 1,
		spear_wooden = 1
	},

	-- TRANSLATORS: Helptext for a worker: Trainer
   helptext = _"Trains the soldiers.",
   animations = animations,
}
