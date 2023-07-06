push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "amazons_hunter_gatherer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Hunter-Gatherer"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      spear_wooden = 1
   },

   programs = {
      hunt = {
         "findobject=type:bob radius:14 attrib:eatable",
         "walk=object",
         "animate=idle duration:1s",
         "callobject=remove",
         "createware=meat",
         "return"
      },
      fish = {
         "findspace=size:any radius:14 resource:resource_fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net priority:50% allow_multiple",
         "mine=resource_fish radius:1",
         "animate=fishing duration:1s",
         "playsound=sound/fisher/fisher_pull_net priority:50% allow_multiple",
         "createware=fish",
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
         hotspot = {31, 26},
         fps = 15,
         frames = 15,
         columns = 5,
         rows = 3
      },
      fishing = {
         hotspot = {16, 31},
         fps = 15,
         frames = 15,
         columns = 5,
         rows = 3
      },
   },
}

pop_textdomain()
