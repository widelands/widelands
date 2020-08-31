push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_pigbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Pig Breeder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 16, 29 }
      },
      walk = {
         hotspot = { 27, 29 },
         fps = 20,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 27, 29 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
