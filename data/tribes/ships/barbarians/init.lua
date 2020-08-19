dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 115, 76 },
      fps = 10
   }
}
add_directional_animation(animations, "sail", dirname, "sail", {115, 76}, 10)


tribes:new_ship_type {
   msgctxt = "barbarians_ship",
   name = "barbarians_ship",
   -- TRANSLATORS: This is the Barbarians' ship's name used in lists of units
   descname = pgettext("barbarians_ship", "Ship"),
   icon = dirname .. "menu.png",
   capacity = 30,
   vision_range = 4,
   animations = animations,
}
