push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "atlanteans_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Fisher"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      fishing_net = 1
   },

   programs = {
      fish = {
         "findspace=size:any radius:7 resource:resource_fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net priority:50% allow_multiple",
         "mine=resource_fish radius:1",
         "animate=fishing duration:9s500ms",
         "playsound=sound/fisher/fisher_pull_net priority:50% allow_multiple",
         "createware=fish",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 2, 20 },
      },
      fishing = {
         hotspot = { 10, 21 },
         fps = 10
      },
      walk = {
         hotspot = { 8, 21 },
         fps = 20,
         directional = true
      },
      walkload = {
         hotspot = { 8, 20 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
