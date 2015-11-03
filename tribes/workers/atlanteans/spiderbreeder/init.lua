dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 8, 22 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {10, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {10, 22}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_spiderbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Spider Breeder"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      milking_tongs = 1
   },

   animations = animations,
}
