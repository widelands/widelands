push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "amazons_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Shipwright"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      hammer = 1
   },

   programs = {
      buildship = {
         "walk=object-or-coords",
         "plant=attrib:amazons_shipconstruction unless object",
         "animate=work duration:500ms",
         "construct",
         "animate=work duration:5s",
         "return"
      },
      buildferry_1 = {
         "findspace=size:swim radius:4 ferry",
      },
      buildferry_2 = {
         "findspace=size:swim radius:4 ferry",
         "walk=coords",
         "animate=work duration:10s",
         "createbob=amazons_ferry",
         "return"
      },
   },

   animation_directory = dirname,
   ware_hotspot = {0, 29},
   spritesheets = {
      walk = {
         directional = true,
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      walkload = {
         directional = true,
         hotspot = {17, 31},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      idle = {
         hotspot = {12, 30},
         fps = 15,
         frames = 40,
         columns = 8,
         rows = 5
      },
      work = {
         hotspot = {16, 38},
         fps = 15,
         frames = 80,
         columns = 10,
         rows = 8,
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = "50%"
         }
      },
   },
}

pop_textdomain()
