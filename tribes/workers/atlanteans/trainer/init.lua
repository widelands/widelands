dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 11, 21 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {14, 21}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {14, 21}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Trainer"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1,
		trident_light = 1,
		tabard = 1
	},

   animations = animations,
}
