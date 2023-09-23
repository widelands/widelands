push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "amazons_woodcutter_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Master Woodcutter"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      harvest_ironwood = {
         -- steps from building to tree: 2-13
         -- falling of ironwood: 1 sec
         -- min. worker time: 2 * 2 * 1.8 + 25 + 1 + 2 = 35.2 sec
         -- max. worker time: 2 * 13 * 1.8 + 25 + 1 + 2 = 74.8 sec
         "findobject=attrib:tree_ironwood radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting priority:100% allow_multiple",
         "animate=hacking duration:25s",
         "playsound=sound/woodcutting/tree_falling priority:100%",
         "callobject=fall",
         "animate=hacking duration:2s",
         "createware=ironwood",
         "return"
      },
      harvest_rubber = {
         -- steps from building to tree: 2-13
         -- falling of rubber: 1 sec
         -- min. worker time: 2 * 2 * 1.8 + 25 + 1 + 2 = 35.2 sec
         -- max. worker time: 2 * 13 * 1.8 + 25 + 1 + 2 = 74.8 sec
         "findobject=attrib:tree_rubber radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting priority:100% allow_multiple",
         "animate=hacking duration:25s",
         "playsound=sound/woodcutting/tree_falling priority:100%",
         "callobject=fall",
         "animate=hacking duration:2s",
         "createware=rubber",
         "return"
      },
      harvest_balsa = {
         -- steps from building to tree: 2-13
         -- falling of balsa: 1 sec
         -- min. worker time: 2 * 2 * 1.8 + 25 + 1 + 2 = 35.2 sec
         -- max. worker time: 2 * 13 * 1.8 + 25 + 1 + 2 = 74.8 sec
         "findobject=attrib:tree_balsa radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting priority:100% allow_multiple",
         "animate=hacking duration:25s",
         "playsound=sound/woodcutting/tree_falling priority:100%",
         "callobject=fall",
         "animate=hacking duration:2s",
         "createware=balsa",
         "return"
      },
      harvest = {
         -- steps from building to tree: 2-13
         -- min. worker time: 2 * 2 * 1.8 + 25 + 2 = 34.2 sec
         -- max. worker time: 2 * 13 * 1.8 + 25 + 2 = 73.8 sec
         -- some of trees delay woodcutting by 1-1.4 sec (aspen, balsa, ironwood, oak, rubber)
         "findobject=attrib:normal_tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting priority:100% allow_multiple",
         "animate=hacking duration:25s",
         "playsound=sound/woodcutting/tree_falling priority:100%",
         "callobject=fall",
         "animate=hacking duration:2s",
         "createware=log",
         "return"
      },
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
