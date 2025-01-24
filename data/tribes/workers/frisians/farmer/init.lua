push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Farmer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         -- steps from building to field: 2-7
         -- avg. step in ideal case (free 12 nearest fields): (4*2+8*3)/12 = 2.667
         -- avg. step in ideal case (free 13 nearest fields): (4*2+8*3+1*4)/13 = 2.769
         -- avg. step in worst case (free 13 furthest fields): (3*4+6*5+3*6+1*7)/13 = 5.154
         -- avg. step in std case (free all fields except road): (4*2+8*3+8*4+6*5+3*6+1*7)/30 = 3.967
         -- min. worker time (12 fields): 2 * 2.667 * 1.8 + 6 + 6 = 21.6  sec
         -- min. worker time:             2 * 2.769 * 1.8 + 6 + 6 = 21.97 sec
         -- max. worker time:             2 * 5.154 * 1.8 + 6 + 6 = 30.55 sec
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=planting duration:6s",
         "plant=attrib:seed_barley",
         "animate=planting duration:6s",
         "return"
      },
      harvest = {
         -- min. worker time (12 fields): 2 * 2.667 * 1.8 + 10 + 4 = 23.6  sec
         -- min. worker time:             2 * 2.769 * 1.8 + 10 + 4 = 23.97 sec
         -- max. worker time:             2 * 5.154 * 1.8 + 10 + 4 = 32.55 sec
         "findobject=attrib:ripe_barley radius:3",
         "walk=object",
         "animate=harvesting duration:10s",
         "callobject=harvest",
         "animate=gathering duration:4s",
         "createware=barley",
         "return"
      }
   },

   ware_hotspot = {0, 20},

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
         hotspot = {16, 26}
      },
      planting = {
         basename = "plant",
         fps = 20,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {13, 25}
      },
      harvesting = {
         basename = "harvest",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {12, 25}
      },
      gathering = {
         basename = "gather",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {12, 18}
      },
   },
   animations = {
      idle = {
         hotspot = {8, 23}
      },
   },
}

pop_textdomain()
