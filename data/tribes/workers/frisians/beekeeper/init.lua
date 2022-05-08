push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_beekeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Beekeeper"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1
   },

   programs = {
      bees = {
         "findobject=attrib:flowering radius:5",
         "walk=object",
         "animate=beeswarm duration:10s",
         "createware=honey",
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
         hotspot = {10, 26}
      },
      beeswarm = {
         directory = dirname,
         basename = "beeswarm",
         fps = 50,
         frames = 100,
         columns = 5,
         rows = 20,
         hotspot = {26, 31}
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
