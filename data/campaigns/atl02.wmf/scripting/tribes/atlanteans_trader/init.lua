push_textdomain("scenario_atl02.wmf")

dirname = "campaigns/atl02.wmf/scripting/tribes/atlanteans_trader/"

wl.Descriptions():new_worker_type {
   name = "atlanteans_trader",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Trader"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 3,

   buildcost = {
      atlanteans_scout = 1,
      atlanteans_horse = 1
   },

   programs = {
      log_trade = {
         "findobject=attrib:tradepole radius:80",
         "createware=log",
         "walk=object",
         "createware=coin_wood",
         "return"
      },
      coal_trade = {
         "findobject=attrib:tradepole radius:80",
         "createware=coal",
         "walk=object",
         "createware=coin_copper",
         "return"
      },
      iron_trade = {
         "findobject=attrib:tradepole radius:80",
         "createware=iron",
         "walk=object",
         "createware=coin_silver",
         "return"
      },
      gold_trade = {
         "findobject=attrib:tradepole radius:80",
         "createware=gold",
         "walk=object",
         "createware=coin_gold",
         "return"
      },
      find_pole = {
         "findobject=attrib:tradepole radius:80",
      }
   },

   spritesheets = {
      idle = {
         frames = 1,
         rows = 1,
         columns = 1,
         hotspot = { 36, 44 }
      },
      walk = {
         fps = 10,
         frames = 10,
         rows = 2,
         columns = 5,
         directional = true,
         hotspot = { 36, 44 }
      },
   }
}

pop_textdomain()
