dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 13, 24 },
   },
   work = {
      pictures = path.list_files(dirname .. "work_??.png"),
      sound_effect = {
            directory = "sound/hammering",
            name = "hammering",
      },
      hotspot = { 12, 27 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 24}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {9, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Shipwright"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      hammer = 1
   },

   programs = {
      buildship = {
         "walk object-or-coords",
         "plant tribe:empire_shipconstruction unless object",
         "play_sound sound/sawmill sawmill 230",
         "animation work 500",
         "construct",
         "animation work 5000",
         "return"
      }
   },

   animations = animations,
}
