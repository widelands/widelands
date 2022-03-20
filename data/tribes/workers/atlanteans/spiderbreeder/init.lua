push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "atlanteans_spiderbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Spider Breeder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      milking_tongs = 1
   },

   animations = {
      idle = {
         hotspot = { 8, 22 }
      },
   },

   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 22 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 22 }
      },
   },
}

pop_textdomain()
