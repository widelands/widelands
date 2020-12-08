push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_smith_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Smith"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 24,
   becomes = "europeans_smith_advanced",
   
   buildcost = {
      europeans_carrier = 1,
      hammer = 2,
      fire_tongs = 1,
      saw = 1
   },

   animations = {
      idle = {
         hotspot = { 10, 21 }
      },
      walk = {
         hotspot = { 8, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
