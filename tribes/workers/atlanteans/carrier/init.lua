dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "idle_\\d+.png"),
      hotspot = { 13, 24 },
      fps=10,
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {8, 25}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 25}, 10)


tribes:new_carrier_type {
   name = "atlanteans_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Carrier",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {},

	-- TRANSLATORS: Helptext for a worker: Carrier
   helptext = _"Carries items along your roads.",
   animations = animations,
}
