dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_geologist",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Geologist"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      hammer = 1
   },

   programs = {
      expedition = {
         "repeatsearch=15 5 search"
      },
      search = {
         "animate=hacking 4000",
         "animate=idle 2000",
         "animate=hacking 4000",
         "findresources"
      }
   },

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
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {10, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      hacking = {
         directory = dirname,
         basename = "hacking",
         hotspot = {10, 23},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
   },
}
