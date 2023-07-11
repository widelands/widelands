push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_forester",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Forester"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:5 avoid:field saplingsearches:7",
         "walk=coords",
         "animate=dig duration:2s",
         "animate=planting duration:1s",
         "plant=attrib:tree_sapling",
         "animate=water duration:2s",
         "return"
      }
   },


   spritesheets = {
      walk = {
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {11, 24}
      },
      dig = {
         fps = 20,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {15, 20}
      },
      water = {
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 20}
      },
      planting = {
         basename = "plant",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {15, 18}
      },
   },
   animations = {
      idle = {
         hotspot = {8, 23}
      },
   },
}

pop_textdomain()
