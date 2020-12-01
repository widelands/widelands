push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_worker_type {
   name = "europeans_fisher_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("europeans_worker", "Advanced Fisher"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      fishing_net = 1
   },

   programs = {
      fish = {
         "findspace=size:any radius:10 resource:resource_fish",
         "walk=coords",
         "mine=resource_fish radius:1",
         "animate=fishing duration:4s",
         "createware=fish",
         "return"
      },
      fish_in_pond = {
         "findobject=attrib:pond_mature radius:10",
         "walk=object",
         "animate=fishing duration:4s",
         "callobject=fall_dry",
         "createware=fish",
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

pop_textdomain()
