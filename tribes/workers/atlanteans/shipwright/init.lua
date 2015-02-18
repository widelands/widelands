dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      sound_effect = {
            directory = "../../../sound/hammering",
            name = "hammering",
      },
      hotspot = { 12, 28 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {12, 28}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {12, 28}, 10)


tribes:new_worker_type {
   name = "atlanteans_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Shipwright",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1,
		hammer = 1
	},

	programs = {
		buildship = {
			"walk object-or-coords",
			"plant tribe:shipconstruction unless object",
			"playFX ../../../sound/sawmill/sawmill 230",
			"animation idle 500",
			"construct",
			"animation idle 5000",
			"return"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Shipwright
   helptext = _"Works at the shipyard and constructs new ships.",
   animations = animations,
}
