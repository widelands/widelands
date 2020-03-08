dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_ranger",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Ranger"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      shovel = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:5 avoid:field saplingsearches:5",
         "walk=coords",
         "animate=dig 3500",
         "animate=plant 2500",
         "plant=attrib:tree_sapling",
         "animate=water 3500",
         "return"
      }
   },

   animations = {
      idle = {
         directory = dirname,
         hotspot = { 11, 20 }
      }
   },
   spritesheets = {
      walk = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 11, 20 }
      },
      dig = {
         directory = dirname,
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 12, 22 }
      },
      plant = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 16, 20 }
      },
      water = {
         directory = dirname,
         fps = 5,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 18, 23 }
      },
   }
}
