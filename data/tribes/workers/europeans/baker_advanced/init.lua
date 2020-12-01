push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_baker_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Advanced Baker"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      bread_paddle = 1
   },

   animations = {
      idle = {
         hotspot = { 5, 26 }
      },
      walk = {
         hotspot = { 14, 27 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 14, 27 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
