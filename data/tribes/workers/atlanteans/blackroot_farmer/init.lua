push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "atlanteans_blackroot_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Blackroot Farmer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         -- steps from building to field: 2-6
         -- avg. step in ideal case (free 4 nearest fields): (4*2)/4 = 2
         -- avg. step in ideal case (free 5 nearest fields): (4*2+1*3)/5 = 2.2
         -- avg. step in worst case (free 5 furthest fields):  (2*4+2*5+1*6)/5 = 4.8
         -- avg. step in std case (free all fields except road): (4*2+4*3+2*4+2*5+1*6)/13 = 3.385
         -- min. worker time (4 fields): 2 * 2   * 1.8 + 9 + 9 = 25.2  sec
         -- min. worker time:            2 * 2.2 * 1.8 + 9 + 9 = 25.92 sec
         -- max. worker time:            2 * 4.8 * 1.8 + 9 + 9 = 35.28 sec
         "findspace=size:any radius:2",
         "walk=coords",
         "animate=planting duration:9s",
         "plant=attrib:seed_blackroot",
         "animate=planting duration:9s",
         "return"
      },
      harvest = {
         -- min. worker time (4 fields): 2 * 2   * 1.8 + 10 + 4 = 21.2  sec
         -- min. worker time:            2 * 2.2 * 1.8 + 10 + 4 = 21.92 sec
         -- max. worker time:            2 * 4.8 * 1.8 + 10 + 4 = 31.28 sec
         "findobject=attrib:ripe_blackroot radius:2",
         "walk=object",
         "animate=harvesting duration:10s",
         "callobject=harvest",
         "animate=gathering duration:4s",
         "createware=blackroot",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 9, 24 }
      },
   },

   spritesheets = {
      planting = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 13, 32 }
      },
      harvesting = {
         fps = 10,
         frames = 15,
         rows = 5,
         columns = 3,
         hotspot = { 13, 32 }
      },
      gathering = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 13, 32 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 13, 24 }
      },
      walkload = {
         basename = "walk",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 13, 24 }
      },
   },
}

pop_textdomain()
