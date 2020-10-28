push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "empire_donkeybreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Donkey Breeder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 19, 23 },
      },
      walk = {
         hotspot = { 28, 26 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 28, 26 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
