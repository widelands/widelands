dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle_\\d+.png" },
      hotspot = { 2, 20 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {8, 21}, 20)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 20}, 10)
add_worker_animations(animations, "fishing", dirname, "fishing", {10, 21}, 10)


tribes:new_worker_type {
   name = "atlanteans_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Fisher",

   buildcost = {
		atlanteans_carrier = 1,
		fishing_net = 1
	},

	programs = {
		fish = {
			"findspace size:any radius:7 resource:fish",
			"walk coords",
			"playFX ../../../sound/fisher/fisher_throw_net 192",
			"mine fish 1",
			"animation fishing 3000",
			"playFX ../../../sound/fisher/fisher_pull_net 192",
			"createware fish",
			"return"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Fisher
   helptext = _"The fisher fishes delicious fish.",
   animations = animations,
}
