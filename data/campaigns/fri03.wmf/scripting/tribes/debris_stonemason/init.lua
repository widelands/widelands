dirname = "tribes/workers/frisians/stonemason/"

push_textdomain("scenario_fri03.wmf")

descriptions:new_worker_type {
   name = "frisians_debris_stonemason",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Debris Stonemason"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      pick = 1
   },

   programs = {
      quarry = {
         "findobject=attrib:debris radius:11",
         "walk=object",
         "animate=hacking duration:20s",
         "callobject=remove",
         "return"
      }
   },

   ware_hotspot = {0, 20},

   animation_directory = dirname,
   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 24}
      },
      walkload = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 26}
      },
      hacking = {
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {17, 31}
      },
   },
   animations = { idle = { hotspot = {8, 23}}},
}

pop_textdomain()
