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
          marble_column = 1,
          quartz = 1,
          diamond = 1
        },
        enhancement_return_on_dismantle = {
          marble = 1,
          quartz = 1,
          diamond = 1
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
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _"mining granite",
         actions = {
            "return=skipped unless economy needs granite",
            "callworker=mine_granite",
            "sleep=duration:16s"
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
