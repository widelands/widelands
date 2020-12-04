push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_weaver_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Weaver"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      needles = 1
   },

   animations = {
      idle = {
         hotspot = { 2, 22 }
      },
      walk = {
         hotspot = { 6, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 9, 22 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
