dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {10, 24}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {10, 25}, 15)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Trainer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      spear = 1,
      tunic = 1,
      ration = 1
   },

   ware_hotspot = {0, 20},
   animations = animations,
}
