dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Scout"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      barbarians_carrier = 1
   },

   programs = {
      scout = {
         "scout=15 75000", -- radius 15, 75 seconds until return
         "return"
      }
   },

   animations = {
      idle = {
         directory = dirname,
         hotspot = { 6, 21 },
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
      }
   }
}
