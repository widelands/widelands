dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "empire_worker",
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
      dig = {
         hotspot = { 5, 22 },
         fps = 5
      },
      planting = {
         basename = "plant",
         hotspot = { 18, 23 },
         fps = 10
      },
      water = {
         hotspot = { 18, 26 },
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
