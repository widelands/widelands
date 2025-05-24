push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "amazons_cassava_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Cassava Farmer"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         -- steps from building to field: 2-7
         -- avg. step in ideal case (free 4 nearest fields): (4*2)/4 = 2
         -- avg. step in worst case (free 4 furthest fields): (3*6+1*7)/4 = 6.25
         -- avg. step in std case (free all fields except road): (4*2+8*3+8*4+6*5+3*6+1*7)/30 = 3.967
         -- min. worker time: 2 * 2 * 1.8 + 6 + 6 = 19.2 sec
         -- max. worker time: 2 * 7 * 1.8 + 6 + 6 = 37.2 sec
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=dig duration:6s",
         "plant=attrib:seed_cassava",
         "animate=planting_harvesting duration:6s",
         "return"
      },
      harvest = {
         -- min. worker time: 2 * 2 * 1.8 + 8 + 6 = 21.2 sec
         -- max. worker time: 2 * 7 * 1.8 + 8 + 6 = 39.2 sec
         "findobject=attrib:ripe_cassava radius:3",
         "walk=object",
         "animate=planting_harvesting duration:8s",
         "callobject=harvest",
         "animate=planting_harvesting duration:6s",
         "createware=cassavaroot",
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
