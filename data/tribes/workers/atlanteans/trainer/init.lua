dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Trainer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      trident_light = 1,
      tabard = 1
   },

   animations = {
      idle = {
         hotspot = { 11, 21 }
      },
      walk = {
         hotspot = { 14, 21 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 14, 21 },
         fps = 10,
         directional = true
      }
   }
}
