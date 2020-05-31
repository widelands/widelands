dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Shipwright"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
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
         "findspace=size:swim radius:5",
      },
      buildferry_2 = {
         "findspace=size:swim radius:5",
         "walk=coords",
         "animate=work 10000",
         "buildferry",
         "return"
      },
   },

   animations = {
      idle = {
         hotspot = { 4, 18 }
      },
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 6, 18 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 20 }
      },
      work = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 12, 22 },
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = 64
         }
      },
   }
}
