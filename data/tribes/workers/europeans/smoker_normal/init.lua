push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_smoker_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Smoker"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      hook_pole = 1
   },

   animations = {
      idle = {
         hotspot = { 17, 22 }
      },
      walk = {
         hotspot = { 20, 22 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 20, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
