dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 4, 23 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {18, 25}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_smelter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Smelter"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1,
		fire_tongs = 1
	},

	-- TRANSLATORS: Helptext for a worker: Smelter
   helptext = pgettext("empire_worker", "Smelts ores into metal."),
   animations = animations,
}
