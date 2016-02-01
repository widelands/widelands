dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 89, 86 },
      fps = 10
   },
   sinking = {
      pictures = path.list_files(dirname .. "sinking_??.png"),
      hotspot = { 89, 86 },
      fps = 7
   }
}
add_worker_animations(animations, "sail", dirname, "sail", {89, 86}, 10)

tribes:new_ship_type {
   name = "atlanteans_ship",
   -- TRANSLATORS: This is the ship's name used in lists of units
   descname = _"Ship",
   capacity = 30,
   vision_range = 4,
   animations = animations,
}
