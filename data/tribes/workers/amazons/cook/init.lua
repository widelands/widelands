push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_worker_type {
   name = "amazons_cook",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Cook"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      kitchen_tools = 1
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
   },
}

pop_textdomain()
