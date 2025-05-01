push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "atlanteans_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Shipwright"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hammer = 1
   },

   programs = {
      buildship = {
         -- worker time: 0.5 + 5 = 5.5 sec
         "walk=object-or-coords",
         "plant=attrib:atlanteans_shipconstruction unless object",
         "playsound=sound/sawmill/sawmill priority:80% allow_multiple",
         "animate=idle duration:500ms",
         "construct",
         "animate=idle duration:5s",
         "return"
      },
      buildwarship = {
         -- worker time: 0.5 + 5 = 5.5 sec
         "walk=object-or-coords",
         "plant=attrib:atlanteans_war_shipconstruction unless object",
         "playsound=sound/sawmill/sawmill priority:80% allow_multiple",
         "animate=idle duration:500ms",
         "construct",
         "animate=idle duration:5s",
         "return"
      },
      buildferry = {
         -- steps from building to water: 2-8
         -- min. worker time: 2 * 2 * 1.8 + 10 = 17.2 sec
         -- max. worker time: 2 * 8 * 1.8 + 10 = 38.8 sec
         "findspace=size:swim radius:5 ferry",
         "walk=coords",
         "animate=idle duration:10s",
         "createbob=atlanteans_ferry",
         "return"
      },
   },

   spritesheets = {
        idle = {
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = "50%"
         },
         fps = 10,
         frames = 92,
         rows = 11,
         columns = 9,
         hotspot = { 11, 27 }
      },
        walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 25 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 24 }
      },
   },
}

pop_textdomain()
