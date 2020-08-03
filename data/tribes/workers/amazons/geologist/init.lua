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
         "animate=hacking duration:4s",
         "animate=idle duration:2s",
         "animate=hacking duration:4s",
         "findresources"
      }
   },

   animation_directory = dirname,
   spritesheets = {
      walk = {
         directional = true,
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      idle = {
         hotspot = {10, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      hacking = {
         hotspot = {10, 23},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
   },
}
