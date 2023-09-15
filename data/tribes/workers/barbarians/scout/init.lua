push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Scout"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      barbarians_carrier = 1
   },

   programs = {
      scout = {
         -- steps from building to beginning of scouting: 2...18
         -- min. worker time: 2 * 1.8 + 75 + 2 * 1.8 = 82.2 sec
         -- max. worker time: 2 * (18 * 1.8 + 75) = 214.8 sec
         "scout=radius:15 duration:1m15s",
         "return"
      },
      targeted_scouting = {
         "walk=object",
         "scout=radius:15 duration:1m15s",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 6, 21 },
      }
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 23 }
      }
   }
}

pop_textdomain()
