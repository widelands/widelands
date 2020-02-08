dirname = path.dirname (__file__)

tribes:new_carrier_type {
   msgctxt = "amazons_worker",
   name = "amazons_carrier",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Carrier"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   programs = {
      fetch_water = {
         "findspace=size:swim radius:8",
         "walk=coords",
         "animate=fetch_water 1000",
         "createware=water",
         "return"
      }
   },

   ware_hotspot = {0, 31},
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {11, 29},
         fps = 10,
         frames = 80,
         columns = 10,
         rows = 8
      },
      fetch_water = {
         directory = dirname,
         basename = "fetch_water",
         hotspot = {10, 22},
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2
      },
      walk = {
         directory = dirname,
         basename = "walk",
         directional = true,
         hotspot = {17, 28},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         directional = true,
         hotspot = {17, 31},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
   },
}
