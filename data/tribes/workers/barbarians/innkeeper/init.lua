dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_innkeeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Innkeeper"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      kitchen_tools = 1
   },

   animations = {
      idle = {
         directory = dirname,
         hotspot = { 9, 24 }
      }
   },
   spritesheets = {
      walk = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 23 }
      },
      walkload = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 24 }
      }
   }
}
