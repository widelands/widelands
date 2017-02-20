dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      sound_effect = {
            directory = "sound/hammering",
            name = "hammering",
      },
      hotspot = { 12, 28 },
      fps = 10
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {12, 28}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {12, 28}, 10)


tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Shipwright"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hammer = 1
   },

   programs = {
      buildship = {
         "walk object-or-coords",
         "plant tribe:atlanteans_shipconstruction unless object",
         "play_sound sound/sawmill sawmill 230",
         "animation idle 500",
         "construct",
         "animation idle 5000",
         "return"
      }
   },

   animations = animations,
}
