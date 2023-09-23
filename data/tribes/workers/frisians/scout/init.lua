push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_scout",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Scout"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1
   },

   programs = {
      scout = {
         -- steps from building to beginning of scouting: 2-18
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

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 23}
      },
      walkload = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {12, 26}
      },
   },
   animations = {
      idle = {
         hotspot = {8, 23}
      },
   },
}

pop_textdomain()
