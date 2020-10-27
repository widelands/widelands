push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "atlanteans_weaponsmith",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Weaponsmith"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      hammer = 1
   },

   animations = {
      idle = {
         hotspot = { 10, 21 }
      },
      walk = {
         hotspot = { 8, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 8, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
