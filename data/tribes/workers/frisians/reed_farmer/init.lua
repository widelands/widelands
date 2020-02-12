dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_reed_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Reed Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      shovel = 1
   },

   programs = {
      plantreed = {
         "findspace=size:any radius:1",
         "walk=coords",
         "animate=dig 2000",
         "animate=planting 1000",
         "plant=attrib:seed_reed",
         "return"
      },
      harvestreed = {
         "findobject=attrib:ripe_reed radius:1",
         "walk=object",
         "animate=harvesting 12000",
         "callobject=harvest",
         "animate=harvesting 1",
         "createware=reed",
         "return"
      },
   },

   ware_hotspot = {0, 20},

   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 24}
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {10, 26}
      },
      planting = {
         directory = dirname,
         basename = "plant",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 18}
      },
      harvesting = {
         directory = dirname,
         basename = "harvest",
         fps = 10,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {13, 21}
      },
      dig = {
         directory = dirname,
         basename = "dig",
         fps = 20,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {14, 20}
      },
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {8, 23}
      },
   },
}
