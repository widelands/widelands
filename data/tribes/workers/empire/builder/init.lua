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
         path = "sound/hammering/hammering",
         priority = 50
      },
      hotspot = { 11, 21 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 23}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {11, 23}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Builder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      hammer = 1
   },

   animations = animations,
}
