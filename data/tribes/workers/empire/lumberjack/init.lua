push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "empire_lumberjack",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Lumberjack"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
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
         "playsound=sound/woodcutting/fast_woodcutting priority:95% allow_multiple",
         "animate=hacking duration:20s",
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
         hotspot = { 6, 24 }
      },
      hacking = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 23, 23 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 9, 22 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 9, 22 }
      },
   },
}

pop_textdomain()
