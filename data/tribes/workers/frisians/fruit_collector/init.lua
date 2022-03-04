push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_fruit_collector",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Fruit Collector"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      basket = 1
   },

   programs = {
      harvest = {
         "findobject=attrib:ripe_bush radius:6",
         "walk=object",
         "animate=gather duration:8s",
         "callobject=harvest",
         "animate=idle duration:1s",
         "createware=fruit",
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
         hotspot = {11, 23}
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
      idle = {
         directory = dirname,
         basename = "idle",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {8, 23}
      },
      gather = {
         directory = dirname,
         basename = "harvest",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {13, 21}
      },
   },
}

pop_textdomain()
