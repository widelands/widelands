dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
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
