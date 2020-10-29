push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "empire_smelter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Smelter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      fire_tongs = 1
   },

   animations = {
      idle = {
         hotspot = { 4, 23 }
      },
      walk = {
         hotspot = { 18, 25 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 22 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
