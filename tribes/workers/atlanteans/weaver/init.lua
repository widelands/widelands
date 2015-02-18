dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 2, 22 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {6, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {9, 22}, 10)


tribes:new_worker_type {
   name = "atlanteans_weaver",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Weaver",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1
	},

	-- TRANSLATORS: Helptext for a worker: Weaver
   helptext = _"Produces cloth for buildings, ships and soldiers.",
   animations = animations,
}
