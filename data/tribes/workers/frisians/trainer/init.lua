dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   }
}
add_walking_animations (animations, "walk", dirname, "walk", {10, 24}, 15)
add_walking_animations (animations, "walkload", dirname, "walkload", {10, 25}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Trainer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      sword_short = 1,
      fur_garment = 1
   },

   ware_hotspot = {0, 20},
   animations = animations,
}
