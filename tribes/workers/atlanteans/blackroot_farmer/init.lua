dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle_\\d+.png" },
      hotspot = { 9, 24 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {13, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {13, 24}, 10)
add_worker_animations(animations, "planting", dirname, "plant", {13, 32}, 10)
add_worker_animations(animations, "harvesting", dirname, "harvest", {13, 32}, 10)
add_worker_animations(animations, "gathering", dirname, "gather", {13, 32}, 10)


tribes:new_worker_type {
   name = "atlanteans_blackroot_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Blackroot Farmer",

   buildcost = {
		atlanteans_carrier = 1,
		shovel = 1
	},

	programs = {
		plant = {
			"findspace size:any radius:2",
			"walk coords",
			"animation planting 4000",
			"plant tribe:blackrootfield_t",
			"animation planting 4000",
			"return"
		},
		harvest = {
			"findobject attrib:blackrootfield radius:2",
			"walk object",
			"animation harvesting 10000",
			"object harvest",
			"animation gathering 2000",
			"createware blackroot",
			"return"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Blackroot Farmer
   helptext = _"Plants and harvests blackroot.",
   animations = animations,
}
