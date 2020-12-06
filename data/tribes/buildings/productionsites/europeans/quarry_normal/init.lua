push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_quarry_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Quarry"),
   icon = dirname .. "menu.png",
   size = "small",

   enhancement = {
        name = "europeans_quarry_advanced",
        enhancement_cost = {
          planks = 1,
          brick = 1,
          grout = 1,
          marble_column = 1
        },
        enhancement_return_on_dismantle = {
          log = 1,
          granite = 1,
          marble = 1
        },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 40, 45 },
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_stonecutter_normal = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=cut_granite on failure fail",
            "call=cut_granite on failure fail",
            "call=cut_marble on failure fail", -- This will find marble 2 out of 6 times
            "call=cut_granite on failure fail",
            "call=cut_granite on failure fail",
            "call=cut_marble on failure fail", -- This will find marble 2 out of 6 times
         }
      },
      cut_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _"quarrying granite",
         actions = {
            "callworker=cut_granite",
            "sleep=duration:15s"
         }
      },
      cut_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying marble because ...
         descname = _"quarrying marble",
         actions = {
            "callworker=cut_marble",
            "sleep=duration:15s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Rocks",
      heading = _"Out of Rocks",
      message = pgettext("europeans_building", "The stonecutter working at this quarry can’t find any rocks in his work area."),
      productivity_threshold = 75
   },
}

pop_textdomain()
