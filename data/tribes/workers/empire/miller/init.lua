push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_miller",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Miller"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 5, 23 }
      },
      walk = {
         hotspot = { 8, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
