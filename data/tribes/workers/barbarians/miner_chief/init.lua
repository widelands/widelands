dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_miner_chief",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Chief Miner"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 28,
   becomes = "barbarians_miner_master",

   animations = {
      idle = {
         directory = dirname,
         hotspot = { 5, 24 }
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
         hotspot = { 10, 25 }
      },
      walkload = {
         directory = dirname,
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 8, 26 }
      }
   }
}
