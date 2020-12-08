push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_smith_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Advanced Smith"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,
   
   buildcost = {
      europeans_carrier = 1,
      hammer = 2,
      fire_tongs = 2,
      saw = 1
   },

   animations = {
      idle = {
         hotspot = { 8, 24 }
      },
      walk = {
         hotspot = { 8, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
