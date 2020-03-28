dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_hunter_gatherer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Hunter Gatherer"),
   helptext_script = dirname .. "helptexts.lua",
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
         "animate=idle 1000",
         "callobject=remove",
         "createware=meat",
         "return"
      },
      fish = {
         "findspace=size:any radius:14 resource:fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net 192",
         "mine=fish 1",
         "animate=fishing 1000",
         "playsound=sound/fisher/fisher_pull_net 192",
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
