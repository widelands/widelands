push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_ferry_type {
   name = "frisians_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Ferry"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   ware_hotspot = {0, 0},
   spritesheets = {
      idle = {
         basename = "idle",
         directory = dirname,
         hotspot = {21, 18},
         fps = 10,
         columns = 5,
         rows = 2,
         frames = 10,
      },
      walk = {
         basename = "sail",
         directory = dirname,
         directional = true,
         hotspot = {21, 18},
         fps = 15,
         columns = 5,
         rows = 3,
         frames = 15,
      },
      walkload = {
         basename = "sail",
         directory = dirname,
         directional = true,
         hotspot = {21, 18},
         fps = 15,
         columns = 5,
         rows = 3,
         frames = 15,
      }
   }
}

pop_textdomain()
