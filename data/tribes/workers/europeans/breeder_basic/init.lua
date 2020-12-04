push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_breeder_basic",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Basic Breeder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,
   
   experience = 16,
   becomes = "europeans_breeder_normal",

   buildcost = {
      europeans_carrier = 1,
      milking_tongs = 1
   },

   animations = {
      idle = {
         hotspot = { 3, 21 },
      },
      walk = {
         hotspot = { 42, 30 },
         fps = 20,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 42, 30 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
