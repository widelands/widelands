push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_carpenter_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Carpenter / Sawyer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      saw = 1
   },

   animations = {
      idle = {
         hotspot = { 5, 31 }
      },
      walk = {
         hotspot = { 16, 31 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 16, 31 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
