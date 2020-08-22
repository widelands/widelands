dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_recruit",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Recruit"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = {
      idle = {
         hotspot = { 10, 30 },
         fps = 5
      },
      walk = {
         hotspot = { 20, 30 },
         fps = 10,
         directional = true
      }
   }
}
