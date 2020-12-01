push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_weaver_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Advanced Weaver"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 4, 24 }
      },
      walk = {
         hotspot = { 6, 26 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 6, 26 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
