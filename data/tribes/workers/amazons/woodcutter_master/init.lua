dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_woodcutter_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Master Woodcutter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      harvest_ironwood = {
         "findobject=attrib:tree_ironwood radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 25000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=hacking 2000",
         "createware=ironwood",
         "return"
      },
      harvest_rubber = {
         "findobject=attrib:tree_rubber radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 25000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=hacking 2000",
         "createware=rubber",
         "return"
      },
      harvest_balsa = {
         "findobject=attrib:tree_balsa radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 25000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=hacking 2000",
         "createware=balsa",
         "return"
      },
      harvest = {
         "findobject=attrib:normal_tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/woodcutting 255",
         "animate=hacking 25000",
         "playsound=sound/woodcutting/tree_falling 130",
         "callobject=fall",
         "animate=hacking 2000",
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
