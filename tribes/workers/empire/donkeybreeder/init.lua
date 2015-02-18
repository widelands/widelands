dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 19, 23 },
      fps = 20
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {28, 26}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {28, 26}, 10)


tribes:new_worker_type {
   name = "empire_donkeybreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Donkey Breeder",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1
	},

	-- TRANSLATORS: Helptext for a worker: Donkey Breeder
   helptext = _"Breeds cute and helpful donkeys for adding them to the transportation system.",
   animations = animations,
}
