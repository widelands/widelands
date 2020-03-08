dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_gamekeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Gamekeeper"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1
   },

   programs = {
      release = {
         "findspace=size:any radius:3",
         "walk=coords",
         "animate=releasein 2000",
         "createbob=bunny chamois deer elk reindeer sheep stag wildboar wisent",
         "animate=releaseout 2000",
         "return"
      }
   },

   animations = {
      idle = {
         directory = dirname,
         hotspot = { 14, 22 }
      },
   },
   spritesheets = {
      walk = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 15, 22 }
      },
      releasein = {
         directory = dirname,
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 15, 22 }
      },
      releaseout = {
         directory = dirname,
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 15, 22 }
      }
   }
}
