push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_carrier_type {
   name = "frisians_reindeer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Reindeer"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   ware_hotspot = {0, 18},

   spritesheets = {
      walkload = {
         directory = dirname,
         basename = "walk",
         fps = 20,
         frames = 20,
         columns = 5,
         rows = 4,
         directional = true,
         hotspot = {15, 31}
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 20,
         frames = 20,
         columns = 5,
         rows = 4,
         directional = true,
         hotspot = {15, 31}
      },
      idle = {
         directory = dirname,
         basename = "idle",
         fps = 20,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {22, 15}
      },
   },
}

pop_textdomain()
