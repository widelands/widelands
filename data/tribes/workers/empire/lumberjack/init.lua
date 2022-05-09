push_textdomain("tribes")

dirname = path.dirname(__file__)

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
