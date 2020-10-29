push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "empire_miner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Miner"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      pick = 1
   },

   experience = 19,
   becomes = "empire_miner_master",

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
