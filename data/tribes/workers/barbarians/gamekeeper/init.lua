push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_gamekeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Gamekeeper"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1
   },

   programs = {
      release = {
         "findspace=size:any radius:3",
         "walk=coords",
         "animate=releasein duration:2s",
         "createbob=bunny chamois deer moose reindeer sheep stag wildboar wisent",
         "animate=releaseout duration:2s",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 14, 22 }
      },
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 15, 22 }
      },
      releasein = {
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 15, 22 }
      },
      releaseout = {
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 15, 22 }
      }
   }
}

pop_textdomain()
