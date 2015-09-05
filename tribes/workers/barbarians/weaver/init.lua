dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 5, 22 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {7, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {11, 28}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_weaver",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Weaver"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1
	},

	-- TRANSLATORS: Helptext for a worker: Weaver
   helptext = pgettext("barbarians_worker", "Produces cloth for ships and soldiers."),
   animations = animations,
}
