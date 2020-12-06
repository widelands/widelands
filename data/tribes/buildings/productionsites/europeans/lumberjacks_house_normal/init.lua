push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_lumberjacks_house_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Lumberjacks House"),
   icon = dirname .. "menu.png",
   size = "small",
   
   enhancement = {
        name = "europeans_lumberjacks_house_advanced",
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
         hotspot = { 37, 44 },
      },
   },

   aihints = {
      prohibited_till = 3600
   },

   working_positions = {
      europeans_lumberjack_normal = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _"felling trees",
         actions = {
            "return=skipped unless economy needs log",
            "callworker=harvest",
            "sleep=duration:15s",
            "produce=log"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Trees",
      heading = _"Out of Trees",
      message = pgettext("europeans_building", "The woodcutter working at this woodcutter’s house can’t find any trees in his work area. You should consider dismantling or destroying the building or building a forester’s house."),
      productivity_threshold = 60
   },
}

pop_textdomain()
