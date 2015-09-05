dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 7, 29 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 30}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {11, 24}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_carpenter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Carpenter"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1,
		saw = 1
	},

	-- TRANSLATORS: Helptext for a worker: Carpenter
   helptext = pgettext("empire_worker", "Works in the sawmill."),
   animations = animations,
}
