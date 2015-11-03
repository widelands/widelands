dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "waiting_???.png"),
      hotspot = { 13, 21 },
      fps = 10
   },
   work = {
      pictures = path.list_files(dirname .. "work_??.png"),
      sound_effect = {
            directory = "sound/hammering",
            name = "hammering",
      },
      hotspot = { 11, 21 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {11, 23}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Builder"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      hammer = 1
   },

   animations = animations,
}
