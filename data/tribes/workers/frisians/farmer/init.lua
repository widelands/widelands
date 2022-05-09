push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Farmer"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=planting duration:6s",
         "plant=attrib:seed_barley",
         "animate=planting duration:6s",
         "return"
      },
      harvest = {
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
         directory = dirname,
         basename = "walk",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 24}
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {16, 26}
      },
      planting = {
         directory = dirname,
         basename = "plant",
         fps = 20,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {13, 25}
      },
      harvesting = {
         directory = dirname,
         basename = "harvest",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {12, 25}
      },
      gathering = {
         directory = dirname,
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
         directory = dirname,
         basename = "idle",
         hotspot = {8, 23}
      },
   },
}

pop_textdomain()
