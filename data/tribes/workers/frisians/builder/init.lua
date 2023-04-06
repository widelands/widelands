push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Builder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      hammer = 1
   },

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 24}
      },
      walkload = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 24}
      },
      idle = {
         fps = 10,
         frames = 64,
         columns = 5,
         rows = 13,
         hotspot = {6, 18}
      },
      work = {
         fps = 10,
         frames = 75,
         columns = 5,
         rows = 15,
         hotspot = {9, 24},
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = "50%"
         },
      },
   },
}

pop_textdomain()
