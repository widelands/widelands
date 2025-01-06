push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "amazons_cocoa_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Cocoa Farmer"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         -- steps from building to field: 2-7
         -- avg. step in ideal case (free 7 nearest fields): (4*2+3*3)/7 = 2.429
         -- avg. step in ideal case (free 8 nearest fields): (4*2+4*3)/8 = 2.5
         -- avg. step in worst case (free 8 furthest fields):  (4*5+3*6+1*7)/8 = 5.625
         -- avg. step in std case (free all fields except road): (4*2+8*3+8*4+6*5+3*6+1*7)/30 = 3.967
         -- min. worker time (7 fields): 2 * 2.429 * 1.8 + 6 + 6 = 20.74 sec
         -- min. worker time:            2 * 2.5   * 1.8 + 6 + 6 = 21    sec
         -- max. worker time:            2 * 5.625 * 1.8 + 6 + 6 = 32.25 sec
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=dig duration:6s",
         "plant=attrib:seed_cocoa",
         "animate=planting_harvesting duration:6s",
         "return"
      },
      harvest = {
         -- min. worker time (7 fields): 2 * 2.429 * 1.8 + 8 + 6 = 22.74 sec
         -- min. worker time:            2 * 2.5   * 1.8 + 8 + 6 = 23    sec
         -- max. worker time:            2 * 5.625 * 1.8 + 8 + 6 = 34.25 sec
         "findobject=attrib:ripe_cocoa radius:3",
         "walk=object",
         "animate=planting_harvesting duration:8s",
         "callobject=harvest",
         "animate=planting_harvesting duration:6s",
         "createware=cocoa_beans",
         "return"
      }
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
         basename = "walk_se",
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      dig = {
         hotspot = {14, 23},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      planting_harvesting = {
         -- one animation that looks good enough both for planting and for harvesting ;)
         hotspot = {11, 18},
         fps = 15,
         frames = 15,
         columns = 5,
         rows = 3
      }
   },
}

pop_textdomain()
