push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_fishers_house_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Fisher’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 60 },
      },
   },

   aihints = {
      needs_water = true,
      prohibited_till = 7200
   },

   working_positions = {
      europeans_fisher_advanced = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing",
         actions = {
            "call=fishing_in_sea",
            "callworker=check_pond",
            "call=fishing_in_pond",
         }
      },
      fishing_in_sea = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing in sea",
         actions = {
            "callworker=fish_in_sea",
            "sleep=duration:32s",
         }
      },
      fishing_in_pond = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing in pond",
         actions = {
            "return=skipped unless economy needs fish",
            "callworker=fish_in_pond",
            "sleep=duration:16s",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fish",
      heading = _"Out of Fish",
      message = pgettext("europeans_building", "The fisher working out of this fisher’s house can’t find any fish in his work area."),
   },
}

pop_textdomain()
