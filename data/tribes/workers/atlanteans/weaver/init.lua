push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "atlanteans_weaver",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Weaver"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 2, 22 }
      },
      walk = {
         hotspot = { 6, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 9, 22 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
