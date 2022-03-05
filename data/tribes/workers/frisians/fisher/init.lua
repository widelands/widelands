push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_worker_type {
   name = "frisians_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Fisher"),
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      fishing_net = 1
   },

   programs = {
      fish = {
         "findspace=size:any radius:8 resource:resource_fish",
         "walk=coords",
         "mine=resource_fish radius:1",
         "animate=fishing duration:3s",
         "createware=fish",
         "return"
      },
      fish_in_pond = {
         "findobject=attrib:pond_mature radius:8",
         "walk=object",
         "animate=fishing duration:3s",
         "callobject=fall_dry",
         "createware=fish",
         "return"
      },
      breed_in_pond = {
         "findobject=attrib:pond_dry radius:8",
         "walk=object",
         "animate=release duration:2s",
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

pop_textdomain()
