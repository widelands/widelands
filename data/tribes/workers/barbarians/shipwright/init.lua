dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 11, 23 },
   },
   work = {
      pictures = path.list_files(dirname .. "work_??.png"),
      sound_effect = {
            directory = "sound/hammering",
            name = "hammering",
      },
      hotspot = { 11, 26 },
      fps = 10
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {9, 24}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {11, 22}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Shipwright"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hammer = 1
   },

   programs = {
      buildship = {
         "walk object-or-coords",
         "plant tribe:barbarians_shipconstruction unless object",
         "play_sound sound/sawmill sawmill 230",
         "animation work 500",
         "construct",
         "animation work 5000",
         "return"
      }
   },

   animations = animations,
}
