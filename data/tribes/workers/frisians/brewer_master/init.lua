push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_worker_type {
   name = "frisians_brewer_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Master Brewer"),
   icon = dirname .. "menu.png",
   vision_range = 2,

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
         hotspot = {10, 23}
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
