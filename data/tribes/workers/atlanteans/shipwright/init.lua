dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      sound_effect = {
         path = "sound/hammering/hammering",
         priority = 64
      },
      hotspot = { 12, 28 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {12, 28}, 10)
add_directional_animation(animations, "walkload", dirname, "walkload", {12, 28}, 10)

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
         "walk=object-or-coords",
         "plant=attrib:shipconstruction unless object",
         "playsound=sound/sawmill/sawmill 230",
         "animate=idle 500",
         "construct",
         "animate=idle 5000",
         "return"
      },
      buildferry_1 = {
         -- checks whether water is available
         "findspace=size:swim radius:5",
      },
      buildferry_2 = {
         "findspace=size:swim radius:5",
         "walk=coords",
         "animate=idle 10000",
         "buildferry",
         "return"
      },
   },

   animations = animations,
}
