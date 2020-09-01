push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "atlanteans_miner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Miner"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      pick = 1
   },

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
