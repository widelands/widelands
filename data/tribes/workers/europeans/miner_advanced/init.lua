push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_miner_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Advanced Miner"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      pick = 1
   },

   animations = {
      idle = {
         hotspot = { 3, 23 }
      },
      walk = {
         hotspot = { 9, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 7, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
