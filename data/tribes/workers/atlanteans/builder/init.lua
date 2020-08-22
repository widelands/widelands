dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Builder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hammer = 1
   },

   animations = {
      idle = {
         basename = "waiting",
         hotspot = { 16, 23 },
         fps = 10,
      },
      work = {
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = 50
         },
         hotspot = { 6, 22 },
         fps = 10,
      },
      walk = {
         hotspot = { 8, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 8, 24 },
         fps = 10,
         directional = true
      }
   }
}
