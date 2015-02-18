dirname = path.dirname(__file__)

animations = {
	idle = {
		pictures = path.list_directory(dirname, "idle_\\d+.png"),
		hotspot = { 115, 100 },
		fps = 10
	}
},
add_worker_animations(animations, "sail", dirname, "sail", {115, 100}, 10)


tribes:new_ship_type {
   name = "ship_empire",
   -- TRANSLATORS: This is the ship's name used in lists of units
   descname = _"Ship",
   capacity = 30,
   vision_range = 4

	-- TRANSLATORS: Helptext for a ship
   helptext = "A ocean-going ship", -- TODO(GunChleoc): Localize when we have help for ships?

   animations = animations,
}
