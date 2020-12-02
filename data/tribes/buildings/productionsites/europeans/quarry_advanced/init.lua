push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_quarry_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Quarry"),
   icon = dirname .. "menu.png",
   size = "small",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 57 },
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_stonecutter_advanced = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=mine_granite on failure fail",
            "call=mine_granite on failure fail",
            "call=mine_marble on failure fail", -- This will find marble 2 out of 6 times
            "call=mine_granite on failure fail",
            "call=mine_granite on failure fail",
            "call=mine_marble on failure fail", -- This will find marble 2 out of 6 times
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _"quarrying granite",
         actions = {
            "callworker=mine_granite",
            "sleep=duration:15s"
         }
      },
      mine_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying marble because ...
         descname = _"quarrying marble",
         actions = {
            "callworker=mine_marble",
            "sleep=duration:15s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Rocks",
      heading = _"Out of Rocks",
      message = pgettext("europeans_building", "The stonemason working at this quarry can’t find any rocks in his work area."),
      productivity_threshold = 75
   },
}

pop_textdomain()
