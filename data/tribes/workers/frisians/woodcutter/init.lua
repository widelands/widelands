push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_woodcutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Woodcutter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      felling_ax = 1
   },

   programs = {
      harvest = {
         -- steps from building to tree: 2-13, min+max average 7.5
         -- min. worker time: 2 * 2 * 1.8 + 20 + 2 = 29.2 sec
         -- max. worker time: 2 * 13 * 1.8 + 20 + 2 = 68.8 sec
         -- avg. worker time: 2 * 7.5 * 1.8 + 20 + 2 = 49 sec
         -- some of trees delay woodcutting by 1-1.4 sec (aspen, balsa, ironwood, oak, rubber)
         "findobject=attrib:tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting priority:100% allow_multiple",
         "animate=hacking duration:20s",
         "playsound=sound/woodcutting/tree_falling priority:100%",
         "callobject=fall",
         "animate=idle duration:2s",
         "createware=log",
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
         hotspot = {16, 23}
      },
      walkload = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {12, 26}
      },
      idle = {
         fps = 10,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {9, 23}
      },
      hacking = {
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 23}
      },
   },
}

pop_textdomain()
