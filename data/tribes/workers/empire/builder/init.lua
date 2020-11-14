push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "empire_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Builder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      hammer = 1
   },

   animations = {
      idle = {
         basename = "waiting",
         hotspot = { 13, 21 },
         fps = 10
      },
      work = {
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = "50%"
         },
         hotspot = { 11, 21 },
         fps = 10
      },
      walk = {
         hotspot = { 11, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 11, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
