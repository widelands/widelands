push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "atlanteans_forester",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Forester"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:5 avoid:field saplingsearches:12",
         "walk=coords",
         "animate=dig duration:3s", -- Play a planting animation
         "animate=planting duration:2s", -- Play a planting animation
         "plant=attrib:tree_sapling",
         "animate=water duration:3s",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 8, 23 },
      },
   },

   spritesheets = {
      dig = {
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 5, 23 }
      },
      planting = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 17, 21 }
      },
      water = {
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 18, 25 }
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
