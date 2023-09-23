push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "amazons_woodcutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Woodcutter"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      felling_ax = 1
   },

   experience = 15,
   becomes = "amazons_woodcutter_master",

   programs = {
      harvest = {
         -- steps from building to tree: 2-13
         -- min. worker time: 2 * 2 * 1.8 + 30 + 2 = 39.2 sec
         -- max. worker time: 2 * 13 * 1.8 + 30 + 2 = 78.8 sec
         -- some of trees delay woodcutting by 1-1.4 sec (aspen, balsa, ironwood, oak, rubber)
         "findobject=attrib:normal_tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting priority:100% allow_multiple",
         "animate=hacking duration:30s",
         "playsound=sound/woodcutting/tree_falling priority:100%",
         "callobject=fall",
         "animate=hacking duration:2s",
         "createware=log",
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
         hotspot = {23, 37},
         fps = 15,
         frames = 15,
         columns = 5,
         rows = 3
      },
   }
}

pop_textdomain()
