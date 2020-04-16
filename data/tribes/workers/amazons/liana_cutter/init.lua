dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_liana_cutter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Liana Cutter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      machete = 1
   },

   programs = {
      cut = {
         "findobject=attrib:tree radius:6",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=work 4000",
         "playsound=sound/woodcutting/tree_falling 130",
         "animate=work 1000",
         "createware=liana",
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
      work = {
         hotspot = {19, 35},
         fps = 15,
         frames = 15,
         columns = 5,
         rows = 3
      },
   }
}
