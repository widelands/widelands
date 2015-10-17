dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 11, 23 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {21, 28}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {17, 27}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_charcoal_burner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Charcoal Burner"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1
	},

   animations = animations,
}
