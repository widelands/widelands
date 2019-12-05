dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 13, 24 },
   },
   work = {
      pictures = path.list_files(dirname .. "work_??.png"),
      sound_effect = {
         path = "sound/hammering/hammering",
         priority = 64
      },
      hotspot = { 12, 27 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 24}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {9, 22}, 10)


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
         "walk=object-or-coords",
         "plant=attrib:shipconstruction unless object",
         "playsound=sound/sawmill/sawmill 230",
         "animate=work 500",
         "construct",
         "animate=work 5000",
         "return"
      },
      buildferry_1 = {
         "findspace=size:swim radius:4",
      },
      buildferry_2 = {
         "findspace=size:swim radius:4",
         "walk=coords",
         "animate=work 10000",
         "buildferry",
         "return"
      },
   },

   animations = animations,
}
