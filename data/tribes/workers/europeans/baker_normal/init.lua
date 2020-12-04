push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_baker_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Baker"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 24,
   becomes = "europeans_baker_advanced",

   animations = {
      idle = {
         hotspot = { 5, 23 }
      },
      walk = {
         hotspot = { 7, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 9, 25 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
