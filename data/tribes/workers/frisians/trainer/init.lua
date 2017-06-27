dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 4, 23 }
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {8, 23}, 10)
add_walking_animations(animations, "walkload", dirname, "walk", {8, 23}, 10)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("frisians_worker", "Trainer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      sword_basic = 1,
      fur_clothes = 1
   },

   animations = animations,
}
