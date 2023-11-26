push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
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
         -- steps from building to water: 2-10, min+max average 6
         -- min. worker time: 2 * 2 * 1.8 + 9.5 = 16.7 sec
         -- max. worker time: 2 * 10 * 1.8 + 9.5 = 45.5 sec
         -- avg. worker time: 2 * 6 * 1.8 + 9.5 = 31.1 sec
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
   },

   spritesheets = {
      fishing = {
         fps = 10,
         frames = 30,
         rows = 6,
         columns = 5,
         hotspot = { 10, 21 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 8, 21 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 20 }
      },
   },
}

pop_textdomain()
