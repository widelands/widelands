dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "wildhorse_idle_sw_\\d+.png" },
      hotspot = { 18, 23 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "wildhorse_walk", {19, 33}, 10)
add_worker_animations(animations, "walkload", dirname, "wildhorse_walk", {19, 33}, 10)


tribes:new_carrier_type {
   name = "atlanteans_horse",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Horse",

	default_target_quantity = 10,
	ware_hotspot = {-2, 12},

	-- TRANSLATORS: Helptext for a worker: Horse
   helptext = _"Horses help to carry items along busy roads. They are reared in a horse farm.",
   animations = animations,
}
