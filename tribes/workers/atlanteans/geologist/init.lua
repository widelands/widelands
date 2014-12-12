dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle_\\d+.png" },
      hotspot = { 10, 23 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {11, 23}, 10)
add_worker_animations(animations, "hacking", dirname, "hacking", {12, 20}, 10)


tribes:new_worker_type {
   name = "atlanteans_geologist",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Geologist",

   buildcost = {
		atlanteans_carrier = 1,
		hammer = 1
	},

	programs = {
		-- Expedition is the main program
		-- The specialized geologist command walks the geologist around his starting
		-- location, executing the search program from time to time.
		expedition = {
			"geologist 15 5 search"
		},
		-- Search program, executed when we have found a place to hack on
		search = {
			"animation hacking 5000",
			"animation idle 2000",
			"playFX ../../../sound/hammering/geologist_hammer 192",
			"animation hacking 3000",
			"geologist-find"
		}
	},

	-- TRANSLATORS: Helptext for a worker: Geologist
   helptext = _"Discovers resources for mining.",
   animations = animations,
}
