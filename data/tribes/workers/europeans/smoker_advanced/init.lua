push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_worker_type {
   name = "europeans_smoker_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("europeans_worker", "Advanced Smoker"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,
   
   buildcost = {
      europeans_carrier = 1,
      hook_pole = 2
   },

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         basename = "walk",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 23}
      },
      walkload = {
         basename = "walkload",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 26}
      },
   },
   animations = {
      idle = {
         basename = "idle",
         hotspot = {8, 23}
      },
   },
}

pop_textdomain()
