dirname = path.dirname (__file__)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Fisher"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      fishing_net = 1
   },

   programs = {
      fish = {
         "findspace=size:any radius:8 resource:fish",
         "walk=coords",
         "mine=fish 1",
         "animate=fishing 3000",
         "createware=fish",
         "return"
      },
      fish_in_pond = {
         "findobject=attrib:pond_mature radius:8",
         "walk=object",
         "animate=fishing 3000",
         "callobject=fall_dry",
         "createware=fish",
         "return"
      },
      breed_in_pond = {
         "findobject=attrib:pond_dry radius:8",
         "walk=object",
         "animate=release 2000",
         "callobject=with_fish",
         "return"
      }
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
         hotspot = {13, 25}
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         fps = 15,
         frames = 10,
         columns = 5,
         rows = 2,
         directional = true,
         hotspot = {13, 26}
      },
      fishing = {
         directory = dirname,
         basename = "fishing",
         fps = 10,
         frames = 30,
         columns = 5,
         rows = 6,
         hotspot = {9, 23}
      },
      release = {
         directory = dirname,
         basename = "release",
         fps = 10,
         frames = 20,
         columns = 5,
         rows = 4,
         hotspot = {15, 20}
      },
   },
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {8, 25}
      },
   },
}
