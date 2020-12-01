push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_fishbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Fish Breeder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      buckets = 1
   },

   programs = {
      breed_in_sea = {
         "findspace=size:any radius:8 breed resource:resource_fish",
         "walk=coords",
         "animate=freeing duration:13s500ms", -- Play a freeing animation
         "breed=resource_fish radius:1",
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
