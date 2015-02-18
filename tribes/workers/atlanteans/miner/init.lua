dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 3, 23 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 24}, 10)


tribes:new_worker_type {
   name = "atlanteans_miner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Miner",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1,
		pick = 1
	},

	-- TRANSLATORS: Helptext for a worker: Miner
   helptext = _"Works deep in the mines to obtain coal, iron, gold or precious stones.",
   animations = animations,
}
