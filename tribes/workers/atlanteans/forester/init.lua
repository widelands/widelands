dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle_\\d+.png" },
      hotspot = { 8, 23 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {10, 23}, 10)
add_worker_animations(animations, "dig", dirname, "dig", {5, 23}, 5)
add_worker_animations(animations, "crop", dirname, "plant", {17, 21}, 10)
add_worker_animations(animations, "water", dirname, "water", {18, 25}, 5)


tribes:new_worker_type {
   name = "atlanteans_forester",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Forester",

   buildcost = {
		atlanteans_carrier = 1,
		shovel = 1
	},

	programs = {
		plant = {
			"findspace size:any radius:5 avoid:seed",
			"walk coords",
			"animation dig 2000 # Play a planting animation",
			"animation crop 1000 # Play a planting animation",
			"plant attrib:tree_sapling",
			"animation water 2000",
			"return"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Forester
   helptext = _"Plants trees.",
   animations = animations,
}
