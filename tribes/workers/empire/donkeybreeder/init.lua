dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 19, 23 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {28, 26}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {28, 26}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_donkeybreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Donkey Breeder"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = animations,
}
