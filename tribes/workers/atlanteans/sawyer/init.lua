dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 5, 31 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {16, 31}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {16, 31}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_sawyer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Sawyer"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1,
		saw = 1
	},

   animations = animations,
}
