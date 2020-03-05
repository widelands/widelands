dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_brewer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Brewer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1
   },

   experience = 10,
   becomes = "barbarians_brewer_master",

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 7, 22 },
      }
   },
   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 12, 23 }
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 11, 24 }
      }
   }
}
