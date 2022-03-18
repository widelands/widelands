push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "atlanteans_fishbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Fish Breeder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      buckets = 1
   },

   programs = {
      breed = {
         "findspace=size:any radius:7 breed resource:resource_fish",
         "walk=coords",
         "animate=freeing duration:13s500ms", -- Play a freeing animation
         "breed=resource_fish radius:1",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 8, 22 }
      },
   },

   spritesheets = {
      freeing = {
         fps = 10,
         frames = 30,
         rows = 6,
         columns = 5,
         hotspot = { 10, 19 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 23 }
      },
   },
}

pop_textdomain()
