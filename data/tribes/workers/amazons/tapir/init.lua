push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_carrier_type {
   name = "amazons_tapir",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Tapir"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   animation_directory = dirname,
   spritesheets = {
      walkload = {
         basename = "walk",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {20, 20}
      },
      walk = {
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {20, 20}
      },
      idle = {
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 22}
      },
   },
}

pop_textdomain()
