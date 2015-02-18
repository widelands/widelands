dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 4, 22 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {8, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 24}, 10)


tribes:new_worker_type {
   name = "atlanteans_miller",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Miller",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1
	},

	-- TRANSLATORS: Helptext for a worker: Miller
   helptext = _"Mills blackroot and corn to produce blackroot flour and corn flour, respectively.",
   animations = animations,
}
