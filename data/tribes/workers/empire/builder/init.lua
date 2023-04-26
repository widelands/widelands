push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
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

   spritesheets = {
      idle = {
         fps = 10,
         frames = 150,
         rows = 13,
         columns = 12,
         hotspot = { 11, 21 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 11, 23 }
      },
      walkload = {
         basename = "walk",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 11, 23 }
      },
      work = {
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = "50%"
         },
         fps = 10,
         frames = 92,
         rows = 11,
         columns = 9,
         hotspot = { 11, 21 }
      },
   },
}

pop_textdomain()
