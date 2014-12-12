dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle_\\d+.png" },
      hotspot = { 10, 23 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {18, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {18, 23}, 10)
add_worker_animations(animations, "planting", dirname, "plant", {13, 32}, 10)
add_worker_animations(animations, "harvesting", dirname, "harvest", {18, 32}, 10)
add_worker_animations(animations, "gathering", dirname, "gather", {10, 34}, 5)


tribes:new_worker_type {
   name = "atlanteans_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Farmer",

   buildcost = {
		atlanteans_carrier = 1,
		scythe = 1
	},

	programs = {
		plant = {
			"findspace size:any radius:2",
			"walk coords",
			"animation planting 4000",
			"plant tribe:cornfield_t",
			"animation planting 4000",
			"return"
		},
		harvest = {
			"findobject attrib:cornfield radius:2",
			"walk object",
			"playFX ../../../sound/farm/scythe 220",
			"animation harvesting 10000",
			"object harvest",
			"animation gathering 4000",
			"createware corn",
			"return"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Farmer
   helptext = _"Plants and harvests cornfields.",
   animations = animations,
}
