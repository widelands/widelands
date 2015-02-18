dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 3, 24 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {9, 25}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {6, 23}, 10)


tribes:new_worker_type {
   name = "barbarians_miner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Miner",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1,
		pick = 1
	},

	experience = 19,
	becomes = "barbarians_miner_chief",

	-- TRANSLATORS: Helptext for a worker: Miner
   helptext = _"Works deep in the mines to obtain coal, iron, gold or granite.",
   animations = animations,
}
