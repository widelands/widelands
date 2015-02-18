dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 11, 23 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {21, 28}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {17, 27}, 10)


tribes:new_worker_type {
   name = "empire_charcoal_burner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Charcoal Burner",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1
	},

	-- TRANSLATORS: Helptext for a worker: Charcoal Burner
   helptext = _"Burns coal.",
   animations = animations,
}
