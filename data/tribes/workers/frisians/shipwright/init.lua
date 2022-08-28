push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Shipwright"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      hammer = 1
   },

   programs = {
      buildship = {
         "walk=object-or-coords",
         "plant=attrib:frisians_shipconstruction unless object",
         "animate=work duration:500ms",
         "construct",
         "animate=work duration:5s",
         "return"
      },
      buildferry_1 = {
         "findspace=size:swim radius:5 ferry",
      },
      buildferry_2 = {
         "findspace=size:swim radius:5 ferry",
         "walk=coords",
         "animate=work duration:10s",
         "createbob=frisians_ferry",
         "return"
      },
   },

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 24}
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 26}
      },
      work = {
         directory = dirname,
         basename = "work",
         fps = 10,
         frames = 75,
         columns = 5,
         rows = 15,
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = "50%"
         },
         hotspot = {9, 24},
      },
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {8, 23}
      },
   },
}

pop_textdomain()
