dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_smoker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Smoker"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
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
