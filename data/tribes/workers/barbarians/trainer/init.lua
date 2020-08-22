dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Trainer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      ax = 1
   },

   animations = {
      idle = {
         hotspot = { 4, 23 }
      }
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 8, 23 }
      }
   }
}
