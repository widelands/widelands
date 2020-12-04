push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_fishbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Fish Breeder and Clay Digger"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      buckets = 1,
      shovel = 1
   },

   programs = {
      dig = {
         "findspace=size:any radius:6",
         "walk=coords",
         "animate=dig duration:8s",
         "plant=attrib:pond_dry",
         "return"
      },
      breed_in_pond = {
         "findobject=attrib:pond_dry radius:6",
         "walk=object",
         "animate=freeing duration:4s",
         "callobject=with_fish",
         "return"
      },
      breed_in_sea = {
         "findspace=size:any radius:8 breed resource:resource_fish",
         "walk=coords",
         "animate=freeing duration:12s", -- Play a freeing animation
         "breed=resource_fish radius:1",
         "return"
      }
   },

   spritesheets = {
      dig = {
         basename = "dig",
         fps = 20,
         frames = 10,
         columns = 5,
         rows = 2,
         hotspot = {15, 20}
      },
   },
   
   animations = {
      idle = {
         hotspot = { 8, 22 }
      },
      freeing = {
         hotspot = { 10, 19 },
         fps = 10
      },
      walk = {
         hotspot = { 11, 23 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
