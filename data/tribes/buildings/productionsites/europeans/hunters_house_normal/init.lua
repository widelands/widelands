push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_hunters_house_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Hunter’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   enhancement = {
        name = "europeans_hunters_house_advanced",
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
         hotspot = { 36, 44 },
      },
   },

   aihints = {
      prohibited_till = 3600
   },

   working_positions = {
      europeans_hunter_normal = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _"hunting",
         actions = {
            "callworker=hunt",
            "sleep=duration:40s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of Game" for a resource
      title = _"No Game",
      -- TRANSLATORS: "Game" means animals that you can hunt
      heading = _"Out of Game",
      -- TRANSLATORS: "game" means animals that you can hunt
      message = pgettext("europeans_building", "The hunter working out of this hunter’s house can’t find any game in his work area."),
      productivity_threshold = 0
   },
}

pop_textdomain()
