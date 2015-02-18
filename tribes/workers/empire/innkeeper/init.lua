dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 4, 26 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {6, 26}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {6, 26}, 10)


tribes:new_worker_type {
   name = "empire_innkeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Innkeeper",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
		empire_carrier = 1,
		kitchen_tools = 1
	},

	-- TRANSLATORS: Helptext for a worker: Innkeeper
   helptext = _"Produces food for miners.",
   animations = animations,
}
