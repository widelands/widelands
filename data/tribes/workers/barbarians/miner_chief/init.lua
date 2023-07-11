push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_miner_chief",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Chief Miner"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 28,
   becomes = "barbarians_miner_master",

   animations = {
      idle = {
         hotspot = { 5, 24 }
      }
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 25 }
      },
      walkload = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 8, 26 }
      }
   }
}

pop_textdomain()
