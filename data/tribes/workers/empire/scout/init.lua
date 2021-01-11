push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "empire_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Scout"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      empire_carrier = 1
   },

   programs = {
      scout = {
         "scout=radius:15 duration:1m15s",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 4, 22 }
      },
      walk = {
         hotspot = { 9, 24 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 9, 24 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
