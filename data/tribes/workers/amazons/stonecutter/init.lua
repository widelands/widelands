push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "amazons_stonecutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Stonecutter"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      pick = 1
   },

   programs = {
      cut_granite = {
         -- steps from building to rock: 2-9, min+max average 5.5
         -- min. worker time: 2 * 2 * 1.8 + 35 = 42.2 sec
         -- max. worker time: 2 * 9 * 1.8 + 35 = 67.4 sec
         -- avg. worker time: 2 * 5.5 * 1.8 + 35 = 54.8 sec
         "findobject=attrib:rocks radius:6",
         "walk=object",
         "animate=hacking duration:35s",
         "callobject=shrink",
         "createware=granite",
         "return"
      }
   },

   animation_directory = dirname,
   ware_hotspot = {0, 29},
   spritesheets = {
      walk = {
         directional = true,
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      walkload = {
         directional = true,
         hotspot = {17, 31},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      idle = {
         basename = "walk_se",
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      hacking = {
         hotspot = {27, 44},
         fps = 15,
         frames = 15,
         columns = 5,
         rows = 3
      },
   }
}

pop_textdomain()
