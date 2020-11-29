push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Trainer"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      tabard = 1,
      armor = 1,
      spear_wooden = 1
   },

   animations = {
      idle = {
         hotspot = { 6, 23 }
      },
      walk = {
         hotspot = { 9, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 9, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
