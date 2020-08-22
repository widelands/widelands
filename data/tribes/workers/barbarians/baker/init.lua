dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_baker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Baker"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      bread_paddle = 1
   },

   animations = {
      idle = {
         hotspot = { 5, 23 },
      }
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 7, 23 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 9, 25 }
      }
   }
}
