push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_carpenter_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Advanced Carpenter / Sawyer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

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
