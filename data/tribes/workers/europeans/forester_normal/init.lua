push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_forester_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Forester"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:8 avoid:field saplingsearches:8",
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
      dig = {
         hotspot = { 5, 23 },
         fps = 5
      },
      planting = {
         basename = "plant",
         pictures = path.list_files(dirname .. "plant_??.png"),
         hotspot = { 17, 21 },
         fps = 10
      },
      water = {
         hotspot = { 18, 25 },
         fps = 5
      },
      walk = {
         hotspot = { 10, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 10, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
