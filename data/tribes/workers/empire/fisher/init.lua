push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Fisher"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      fishing_rod = 1
   },

   programs = {
      fish = {
         "findspace=size:any radius:7 resource:fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net priority:50% allow_multiple",
         "mine=resource_fish radius:1",
         "animate=fishing duration:10s", -- Play a fishing animation
         "playsound=sound/fisher/fisher_pull_net priority:50% allow_multiple",
         "createware=fish",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 7, 38 },
      },
      fishing = {
         hotspot = { 9, 39 },
         fps = 10,
      },
      walk = {
         hotspot = { 10, 38 },
         fps = 20,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 10, 38 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
