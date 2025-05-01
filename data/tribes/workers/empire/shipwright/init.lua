push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "empire_shipwright",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Shipwright"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      hammer = 1
   },

   programs = {
      buildship = {
         -- worker time: 0.5 + 5 = 5.5 sec
         "walk=object-or-coords",
         "plant=attrib:empire_shipconstruction unless object",
         "playsound=sound/sawmill/sawmill priority:80% allow_multiple",
         "animate=work duration:500ms",
         "construct",
         "animate=work duration:5s",
         "return"
      },
      buildwarship = {
         -- worker time: 0.5 + 5 = 5.5 sec
         "walk=object-or-coords",
         "plant=attrib:empire_war_shipconstruction unless object",
         "playsound=sound/sawmill/sawmill priority:80% allow_multiple",
         "animate=work duration:500ms",
         "construct",
         "animate=work duration:5s",
         "return"
      },
      buildferry = {
         -- steps from building to water: 2-8
         -- min. worker time: 2 * 2 * 1.8 + 10 = 17.2 sec
         -- max. worker time: 2 * 8 * 1.8 + 10 = 38.8 sec
         "findspace=size:swim radius:5 ferry",
         "walk=coords",
         "animate=work duration:10s",
         "createbob=empire_ferry",
         "return"
      },
   },

   animations = {
      idle = {
         hotspot = { 13, 24 },
      },
   },

   spritesheets = {
      work = {
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = "50%"
         },
         fps = 10,
         frames = 92,
         rows = 11,
         columns = 9,
         hotspot = { 12, 27 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 11, 24 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 9, 22 }
      },
   }
}

pop_textdomain()
