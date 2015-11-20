dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "waiting_???.png"),
      hotspot = { 16, 23 },
      fps=10,
   },
   work = {
      pictures = path.list_files(dirname .. "work_??.png"),
      sound_effect = {
            directory = "sound/hammering",
            name = "hammering",
      },
      hotspot = { 6, 22 },
      fps=10,
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {8, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {8, 24}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Builder"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hammer = 1
   },

   animations = animations,
}
