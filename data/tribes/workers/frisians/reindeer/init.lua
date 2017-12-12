dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = { 33, 44 },
      fps = 20
   }
}

add_walking_animations (animations, "walk", dirname, "walk", {33, 44}, 20)
add_walking_animations (animations, "walkload", dirname, "walk", {33, 44}, 20)

tribes:new_carrier_type {
   msgctxt = "frisians_worker",
   name = "frisians_reindeer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Reindeer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   default_target_quantity = 10,
   ware_hotspot = { 0, 18 },

   animations = animations,
}
