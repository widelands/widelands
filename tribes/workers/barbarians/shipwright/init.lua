dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 11, 23 },
      fps = 10
   },
   work = {
      pictures = path.list_directory(dirname, "work_\\d+.png"),
      sfx = "../../../sound/hammering/hammering",
      hotspot = { 11, 26 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {9, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {11, 22}, 10)


tribes:new_worker_type {
   name = "barbarians_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Shipwright",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1,
		hammer = 1
	},

	programs = {
		buildship = {
			"walk object-or-coords",
			"plant tribe:shipconstruction unless object",
			"playFX ../../../sound/sawmill/sawmill 230",
			"animation work 500",
			"construct",
			"animation work 5000",
			"return"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Shipwright
   helptext = _"Works at the shipyard and constructs new ships.",
   animations = animations,
}
