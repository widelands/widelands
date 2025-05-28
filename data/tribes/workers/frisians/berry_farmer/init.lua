push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_berry_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Berry Farmer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         -- steps from building to field: 2-7
         -- min. worker time: 2 * 2 * 1.8 + 2 + 1 + 2 = 12.2 sec
         -- max. worker time: 2 * 7 * 1.8 + 2 + 1 + 2 = 30.2 sec
         "findspace=size:any radius:4",
         "walk=coords",
         "animate=dig duration:2s",
         "animate=planting duration:1s",
         "plant=attrib:seed_berrybush",
         "animate=water duration:2s",
         "return"
      }
   },


   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 24}
      },
      dig = {
         fps = 20,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {15, 20}
      },
      water = {
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 20}
      },
      planting = {
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
         hotspot = {8, 23}
      },
   },
}

pop_textdomain()
