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
            "call=mine_marble on failure fail", -- This will find marble 2 out of 8 times
            "call=mine_quartz on failure fail", -- This will find quartz 1 out of 8 times
            "call=mine_granite on failure fail",
            "call=mine_granite on failure fail",
            "call=mine_marble on failure fail", -- This will find marble 2 out of 8 times
            "call=mine_diamond on failure fail", -- This will find diamond 1 out of 8 times
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _"mining granite",
         actions = {
            "callworker=mine_granite",
            "sleep=duration:15s"
         }
      },
      mine_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining marble because ...
         descname = _"mining marble",
         actions = {
            "callworker=mine_marble",
            "sleep=duration:15s"
         }
      },
      mine_quartz = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining quartz because ...
         descname = _"mining quartz",
         actions = {
            "callworker=mine_quartz",
            "sleep=duration:15s"
         }
      },
      mine_diamond = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining diamond because ...
         descname = _"mining diamond",
         actions = {
            "callworker=mine_diamond",
            "sleep=duration:15s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Rocks",
      heading = _"Out of Rocks",
      message = pgettext("europeans_building", "The stonecutter working at this quarry can’t find any stone in his work area."),
      productivity_threshold = 75
   },
}

pop_textdomain()
