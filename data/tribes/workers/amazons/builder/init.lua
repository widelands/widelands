dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Builder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      hammer = 1
   },

   ware_hotspot = {0, 29},
   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         directional = true,
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         directional = true,
         hotspot = {17, 31},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {12, 30},
         fps = 15,
         frames = 40,
         columns = 8,
         rows = 5
      },
      work = {
         directory = dirname,
         basename = "work",
         hotspot = {16, 38},
         fps = 15,
         frames = 80,
         columns = 10,
         rows = 8,
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = 64
         }
      },
   },
}
