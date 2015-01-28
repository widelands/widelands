dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "waiting_\\d+.png" },
      hotspot = { 8, 22 }
   },
   freeing = {
      pictures = { dirname .. "freeing_\\d+.png" },
      hotspot = { 10, 19 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 23}, 20)
add_worker_animations(animations, "walkload", dirname, "walk", {11, 23}, 20)


tribes:new_worker_type {
   name = "atlanteans_fishbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Fish Breeder",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1,
		bucket = 1
	},

	programs = {
		breed = {
			"findspace size:any radius:7 breed resource:fish",
			"walk coords",
			"animation freeing 3000 # Play a freeing animation",
			"breed fish 1",
			"return"
		}
	}

	-- TRANSLATORS: Helptext for a worker: Fish Breeder
   helptext = _"Breeds fish.",
   animations = animations,
}
