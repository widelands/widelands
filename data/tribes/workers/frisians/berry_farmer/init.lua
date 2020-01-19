dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_berry_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Berry Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:4",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:seed_berrybush",
         "animate=water 2000",
         "return"
      }
   },


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
      dig = {
         directory = dirname,
         basename = "dig",
         fps = 20,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {15, 20}
      },
      water = {
         directory = dirname,
         basename = "water",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 20}
      },
      planting = {
         directory = dirname,
         basename = "plant",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {15, 18}
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
