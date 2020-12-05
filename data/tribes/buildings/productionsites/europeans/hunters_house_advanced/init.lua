push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_hunters_house_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Hunter’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 54, 55 },
      },
   },

   aihints = {
      prohibited_till = 7200
   },

   working_positions = {
      europeans_hunter_advanced = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _"hunting",
         actions = {
            "return=skipped unless economy needs meat",
            "callworker=hunt",
            "sleep=duration:16s"
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
