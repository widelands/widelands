dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Trainer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      sword_short = 1,
      fur_garment = 1
   },

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 24}
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 25}
      },
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {8, 23}
      },
   },
}
