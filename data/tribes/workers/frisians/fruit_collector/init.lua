push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_fruit_collector",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Fruit Collector"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      basket = 1
   },

   programs = {
      harvest = {
         -- steps from building to field: 2-9
         -- min. worker time: 2 * 2 * 1.8 + 8 + 1 = 16.2 sec
         -- max. worker time: 2 * 9 * 1.8 + 8 + 1 = 41.4 sec
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
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 23}
      },
      walkload = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 26}
      },
      idle = {
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {8, 23}
      },
      gather = {
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
