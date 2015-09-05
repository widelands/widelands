dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 6, 22 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {8, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {9, 23}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_charcoal_burner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Charcoal Burner"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		barbarians_carrier = 1
	},

	-- TRANSLATORS: Helptext for a worker: Charcoal Burner
   helptext = pgettext("barbarians_worker", "Burns coal."),
   animations = animations,
}
