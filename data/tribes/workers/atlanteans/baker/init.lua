push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "atlanteans_baker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Baker"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      bread_paddle = 1
   },

   animations = {
      idle = {
         hotspot = { 5, 23 }
      },
      walk = {
         hotspot = { 7, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 9, 25 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
