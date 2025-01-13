push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_reed_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Reed Farmer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      shovel = 1
   },

   programs = {
      plantreed = {
         -- steps from building to field: 2-4
         -- avg. step in ideal case (free 3 nearest fields): (2*2+1*3)/3 = 2.333
         -- avg. step in worst case (free 3 furthest fields):  (2*3+1*4)/3 = 3.333
         -- avg. step in std case (free all fields): (2*2+2*3+1*4)/5 = 2.8
         -- min. worker time: 2 * 2.333 * 1.8 + 2 + 1 = 11.4 sec
         -- max. worker time: 2 * 3.333 * 1.8 + 2 + 1 = 15   sec
         "findspace=size:any radius:1",
         "walk=coords",
         "animate=dig duration:2s",
         "animate=planting duration:1s",
         "plant=attrib:seed_reed",
         "return"
      },
      harvestreed = {
         -- min. worker time: 2 * 2.333 * 1.8 + 6 + 6 = 20.4 sec
         -- max. worker time: 2 * 3.333 * 1.8 + 6 + 6 = 24   sec
         "findobject=attrib:ripe_reed radius:1",
         "walk=object",
         "animate=harvesting duration:6s",
         "callobject=harvest",
         "animate=harvesting duration:6s",
         "createware=reed",
         "return"
      },
   },

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 24}
      },
      walkload = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 26}
      },
      planting = {
         basename = "plant",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 18}
      },
      harvesting = {
         basename = "harvest",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {13, 21}
      },
      dig = {
         fps = 20,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 20}
      },
   },
   animations = {
      idle = {
         hotspot = {8, 23}
      },
   },
}

pop_textdomain()
