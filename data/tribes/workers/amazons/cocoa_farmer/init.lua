dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {8, 23},
   },
   planting = {
      pictures = path.list_files (dirname .. "plant_??.png"),
      hotspot = { 13, 25 },
      fps = 20
   },
   harvesting = {
      pictures = path.list_files (dirname .. "harvest_??.png"),
      hotspot = { 12, 25 },
      fps = 10
   },
   gathering = {
      pictures = path.list_files (dirname .. "gather_??.png"),
      hotspot = { 12, 18 },
      fps = 10
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {11, 24}, 15)
add_directional_animation(animations, "walkload", dirname, "walkload", {16, 26}, 15)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_cocoa_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Cocoa Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      amazons_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:3 space",
         "walk=coords",
         "animate=dig 6000",
         "plant=attrib:seed_cocoa",
         "animate=planting_harvesting 6000",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_cocoa radius:3",
         "walk=object",
         "animate=planting_harvesting 8000",
         "callobject=harvest",
         "animate=planting_harvesting 6000",
         "createware=cocoa_beans",
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
      dig = {
         hotspot = {14, 23},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      planting_harvesting = {
         -- one animation that looks good enough both for planting and for harvesting ;)
         hotspot = {11, 18},
         fps = 15,
         frames = 15,
         columns = 5,
         rows = 3
      }
   },
}
