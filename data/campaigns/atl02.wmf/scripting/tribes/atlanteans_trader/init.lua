push_textdomain("scenario_atl02.wmf")

dirname = "campaigns/atl02.wmf/scripting/tribes/atlanteans_trader/"

tribes:new_worker_type {
   name = "atlanteans_trader",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Trader"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      atlanteans_carrier = 1,
      atlanteans_horse = 1
   },

   programs = {
      trade = {
         "findobject=attrib:tradepole radius:100",
         "walk=object",
         "return"
      },
      find_pole = {
         "findobject=attrib:tradepole radius:100",
      }
   },

   animations = {
      idle = {
         hotspot = { 4, 22 }
      },
      walk = {
         hotspot = { 9, 25 },
         fps = 10,
         directional = true
      },
   }
}

pop_textdomain()
