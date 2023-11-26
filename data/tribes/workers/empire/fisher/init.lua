push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
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
         -- steps from building to water: 2-10, min+max average 6
         -- min. worker time: 2 * 2 * 1.8 + 10 = 17.2 sec
         -- max. worker time: 2 * 10 * 1.8 + 10 = 46 sec
         -- avg. worker time: 2 * 6 * 1.8 + 10 = 31.6 sec
         "findspace=size:any radius:7 resource:resource_fish",
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
   },

   spritesheets = {
      fishing = {
         fps = 10,
         frames = 30,
         rows = 6,
         columns = 5,
         hotspot = { 9, 39 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 10, 38 }
      },
      walkload = {
         basename = "walk",
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 10, 38 }
      },
   },
}

pop_textdomain()
