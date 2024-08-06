push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "amazons_jungle_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Jungle Master"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      plant_ironwood = {
         -- steps from building to tree: 2-8, mean 4.94
         -- min. worker time: 2 * 2 * 1.8 + 2 + 1 + 1 = 11.2 sec
         -- max. worker time: 2 * 8 * 1.8 + 2 + 1 + 1 = 32.8 sec
         -- mean worker time: 2 * 4.94 * 1.8 + 2 + 1 + 1 = 21.784 sec
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig duration:2s",
         "animate=planting duration:1s",
         "plant=attrib:tree_ironwood_sapling",
         "animate=planting duration:1s",
         "return"
      },
      plant_rubber = {
         -- steps from building to tree: 2-8, mean 4.94
         -- min. worker time: 2 * 2 * 1.8 + 2 + 1 + 1 = 11.2 sec
         -- max. worker time: 2 * 8 * 1.8 + 2 + 1 + 1 = 32.8 sec
         -- mean worker time: 2 * 4.94 * 1.8 + 2 + 1 + 1 = 21.784 sec
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig duration:2s",
         "animate=planting duration:1s",
         "plant=attrib:tree_rubber_sapling",
         "animate=planting duration:1s",
         "return"
      },
      plant_balsa = {
         -- steps from building to tree: 2-8, mean 4.94
         -- min. worker time: 2 * 2 * 1.8 + 2 + 1 + 1 = 11.2 sec
         -- max. worker time: 2 * 8 * 1.8 + 2 + 1 + 1 = 32.8 sec
         -- mean worker time: 2 * 4.94 * 1.8 + 2 + 1 + 1 = 21.784 sec
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig duration:2s",
         "animate=planting duration:1s",
         "plant=attrib:tree_balsa_sapling",
         "animate=planting duration:1s",
         "return"
      },
      check = {
         "findspace=size:any radius:6 terraform:amazons",
      },
      terraform = {
         "findspace=size:any radius:6 terraform:amazons",
         "walk=coords",
         "animate=dig duration:2s",
         "terraform=amazons",
         "animate=dig duration:2s",
         "return"
      },
      plant = {
         -- steps from building to tree: 2-9, mean 5.58
         -- min. worker time: 2 * 2 * 1.8 + 2 + 1 + 1 = 11.2 sec
         -- max. worker time: 2 * 9 * 1.8 + 2 + 1 + 1 = 36.4 sec
         -- mean worker time: 2 * 5.58 * 1.8 + 2 + 1 + 1 = 24.088 sec
         "findspace=size:any radius:6 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig duration:2s",
         "animate=planting duration:1s",
         "plant=attrib:tree_pole",
         "animate=planting duration:1s",
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

pop_textdomain()
