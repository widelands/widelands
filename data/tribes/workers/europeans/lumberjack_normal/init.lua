push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_lumberjack_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Lumberjack / Woodcutter"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      saw = 1
   },

   programs = {
      harvest = {
         "findobject=attrib:tree radius:14",
         "walk=object",
         "playsound=sound/europeans/saw/sawing priority:80% allow_multiple",
         "animate=sawing duration:12s",
         "playsound=sound/woodcutting/tree_falling priority:100%",
         "callobject=fall",
         "animate=idle duration:2s",
         "createware=log",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 8, 22 }
      },
      sawing = {
         hotspot = { 22, 19 },
         fps = 10
      },
      walk = {
         hotspot = { 16, 31 },
         fps = 10,
         directional = true
      },
      walkload = {
         hotspot = { 13, 29 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
