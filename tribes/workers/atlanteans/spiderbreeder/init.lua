dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 8, 22 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 22}, 10)


tribes:new_worker_type {
   name = "atlanteans_spiderbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Spider Breeder",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		atlanteans_carrier = 1,
		milking_tongs = 1
	},

	-- TRANSLATORS: Helptext for a worker: Spider Breeder
   helptext = _"Breeds spiders for silk.",
   animations = animations,
}
