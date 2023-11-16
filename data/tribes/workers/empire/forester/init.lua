push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "empire_forester",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Forester"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         -- steps from building to tree: 2-8, mean 4.94
         -- min. worker time: 2 * 2 * 1.8 + 2.5 + 1.5 + 2 = 13.2 sec
         -- max. worker time: 2 * 8 * 1.8 + 2.5 + 1.5 + 2 = 34.8 sec
         -- mean worker time: 2 * 4.94 * 1.8 + 2.5 + 1.5 + 2 = 23.784 sec
         "findspace=size:any radius:5 avoid:field saplingsearches:8",
         "walk=coords",
         "animate=dig duration:2s500ms",
         "animate=planting duration:1s500ms",
         "plant=attrib:tree_sapling",
         "animate=water duration:2s",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 3, 23 }
      },
   },

   spritesheets = {
      dig = {
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 5, 22 }
      },
      planting = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 18, 23 }
      },
      water = {
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 18, 26 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 23 }
      },
      walkload = {
         basename = "walk",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 10, 23 }
      },
   },
}

pop_textdomain()
