dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "amazons_worker",
   name = "amazons_jungle_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Jungle Master"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      plant_ironwood = {
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_ironwood_sapling",
         "animate=planting 1000",
         "return"
      },
      plant_rubber = {
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_rubber_sapling",
         "animate=planting 1000",
         "return"
      },
      plant_balsa = {
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_balsa_sapling",
         "animate=planting 1000",
         "return"
      },
      check = {
         "findspace=size:any radius:6 terraform",
      },
      terraform = {
         "findspace=size:any radius:6 terraform",
         "walk=coords",
         "animate=dig 2000",
         "terraform",
         "animate=dig 2000",
         "return"
      },
      plant = {
         "findspace=size:any radius:6 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:tree_pole",
         "animate=planting 1000",
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
      planting = {
         hotspot = {11, 18},
         fps = 15,
         frames = 15,
         columns = 5,
         rows = 3
      },
   }
}
