push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_carpenter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Carpenter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      saw = 1
   },

   animations = {
      idle = {
         hotspot = { 7, 29 }
      },
      walk = {
         hotspot = { 11, 30 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 11, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
