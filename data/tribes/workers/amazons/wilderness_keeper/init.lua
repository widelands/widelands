dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_wilderness_keeper",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Wildernesskeeper"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1
   },

   programs = {
      release_game = {
         "findobject=attrib:tree radius:10",
         "walk=object",
         "animate=release 4181",
         "createbob=bunny badger fox lynx marten wildboar",
         "return"
      },
      breed_fish = {
         "findspace=size:swim radius:7",
         "walk=coords",
         "animate=release 12545",
         "breed=fish 1",
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
      idle = {
         basename = "walk_se",
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      release = {
         hotspot = {14, 25},
         -- I apologize for the unusual frame durations in this animation…
         fps = 11,
         frames = 23,
         columns = 6,
         rows = 4
      },
   },
}
