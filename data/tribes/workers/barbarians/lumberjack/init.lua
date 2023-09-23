push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_lumberjack",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Lumberjack"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      felling_ax = 1
   },

   programs = {
      harvest = {
         -- steps from building to tree: 2...13
         -- min. worker time: 2 * 2 * 1.8 + 15 + 2 = 24.2 sec
         -- max. worker time: 2 * 13 * 1.8 + 15 + 2 = 63.8 sec
         -- some of trees delay woodcutting by 1-1.4 sec (aspen, balsa, ironwood, oak, rubber)
         "findobject=attrib:tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting priority:100% allow_multiple",
         "animate=hack duration:15s",
         "playsound=sound/woodcutting/tree_falling priority:100%",
         "callobject=fall",
         "animate=idle duration:2s",
         "createware=log",
         "return"
      }
   },

   spritesheets = {
      idle = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 5, 21 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 20 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 19 }
      },
      hack = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 19, 15 }
      },
   }
}

pop_textdomain()
