dirname = path.dirname(__file__)

-- TODO(GunChleoc): The animations were taken from the brewer.
animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 8, 24 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {14, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_vintner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Vintner"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1
	},

   animations = animations,
}
