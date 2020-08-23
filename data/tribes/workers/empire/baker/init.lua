dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_baker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Baker"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      bread_paddle = 1
   },

   animations = {
      idle = {
         hotspot = { 5, 26 }
      },
      walk = {
         hotspot = { 14, 27 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 14, 27 },
         fps = 10,
         directional = true
      }
   }
}
