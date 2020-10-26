push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "barbarians_cattlebreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Cattle Breeder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1
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
