push_textdomain("scenario_atl02.wmf")

local dirname = "campaigns/atl02.wmf/scripting/tribes/atlanteans_initiate/"

wl.Descriptions():new_worker_type {
   name = "atlanteans_initiate",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Satul’s Initiate"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      atlanteans_carrier = 1,
      tabard_golden = 1,
      diamond = 1
   },

   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 12, 27 }
      },
      walk = {
         fps = 10,
         frames = 10,
         columns = 10,
         rows = 1,
         directional = true,
         hotspot = { 12, 27 }
      },
   }
}

pop_textdomain()
