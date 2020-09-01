push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "atlanteans_fishbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Fish Breeder"),
   helptext_script = dirname .. "helptexts.lua",
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      buckets = 1
   },

   programs = {
      breed = {
         "findspace=size:any radius:7 breed resource:fish",
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
