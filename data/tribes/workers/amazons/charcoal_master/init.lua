push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "amazons_charcoal_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Charcoal Master"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   animation_directory = dirname,
   ware_hotspot = {0, 28},
   spritesheets = {
      walk = {
         directional = true,
         hotspot = {19, 33},
         fps = 15,
         frames = 30,
         columns = 10,
         rows = 3
      },
      walkload = {
         directional = true,
         hotspot = {19, 33},
         fps = 15,
         frames = 30,
         columns = 10,
         rows = 3
      },
      idle = {
         basename = "walk_se",
         hotspot = {19, 33},
         fps = 15,
         frames = 30,
         columns = 10,
         rows = 3
      },
   },
}

pop_textdomain()
