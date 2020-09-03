push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "atlanteans_horsebreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Horse Breeder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 8, 23 },
      },
      walk = {
         hotspot = { 35, 28 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
