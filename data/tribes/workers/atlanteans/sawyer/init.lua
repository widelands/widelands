dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_sawyer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Sawyer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
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
