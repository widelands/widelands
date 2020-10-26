push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "barbarians_hunters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Hunter’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4,
      granite = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 44, 44 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 44, 44 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 44, 44 },
      },
   },

   aihints = {
      collects_ware_from_map = "meat",
      prohibited_till = 480,
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      barbarians_hunter = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _"hunting",
         actions = {
            "callworker=hunt",
            "sleep=duration:35s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of Game" for a resource
      title = _"No Game",
      -- TRANSLATORS: "Game" means animals that you can hunt
      heading = _"Out of Game",
      -- TRANSLATORS: "game" means animals that you can hunt
      message = pgettext("barbarians_building", "The hunter working out of this hunter’s hut can’t find any game in his work area. Remember that you can build a gamekeeper’s hut to release more game into the wild."),
      productivity_threshold = 33
   },
}

pop_textdomain()
