dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 5, 23 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {7, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {9, 25}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_baker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Baker"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1,
		bread_paddle = 1
	},

   animations = animations,
}
