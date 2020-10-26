push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "barbarians_blacksmith_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Master Blacksmith"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = {
      idle = {
         hotspot = { 3, 22 },
      }
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 6, 22 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 9, 23 }
      }
   }
}

pop_textdomain()
