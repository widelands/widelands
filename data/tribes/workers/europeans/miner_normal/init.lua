push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_miner_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Miner"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 24,
   becomes = "europeans_miner_advanced",

   animations = {
      idle = {
         hotspot = { 3, 23 }
      },
      walk = {
         hotspot = { 10, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 10, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
