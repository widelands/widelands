push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_worker_type {
   name = "barbarians_fisher",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Fisher"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      fishing_rod = 1
   },

   programs = {
      fish = {
         -- steps from building to water: 2-10, min+max average 6
         -- min. worker time: 2 * 2 * 1.8 + 10.5 = 17.7 sec
         -- max. worker time: 2 * 10 * 1.8 + 10.5 = 46.5 sec
         -- avg. worker time: 2 * 6 * 1.8 + 10.5 = 32.1 sec
         "findspace=size:any radius:7 resource:resource_fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net priority:50% allow_multiple",
         "mine=resource_fish radius:1",
         "animate=fish duration:10s500ms",
         "playsound=sound/fisher/fisher_pull_net priority:50% allow_multiple",
         "createware=fish",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 7, 33 },
      },
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 32 }
      },
      -- TODO(GunChleoc): Needs walkload animation, or some ware hotspot/sizing magic.
      fish = {
         fps = 10,
         frames = 30,
         rows = 6,
         columns = 5,
         hotspot = { 7, 33 }
      }
   }
}

pop_textdomain()
