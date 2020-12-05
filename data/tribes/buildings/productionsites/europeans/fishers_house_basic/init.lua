push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_fishers_house_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Fisher’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   enhancement = {
        name = "europeans_fishers_house_normal",
        enhancement_cost = {
          brick = 1,
          grout = 1,
          spidercloth = 1,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 1,
          quartz = 1,
        },
   },

   buildcost = {
      planks = 2,
      reed = 2
   },
   return_on_dismantle = {
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 52 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 39, 52 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 39, 52 },
      },
   },

   aihints = {
      collects_ware_from_map = "fish",
      needs_water = true
   },

   working_positions = {
      europeans_fisher_basic = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing",
         actions = {
            "call=fishing_in_sea"
         }
      },
      fishing_in_sea = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing in sea",
         actions = {
            "return=skipped unless economy needs fish",
            "callworker=fish_in_sea",
            "sleep=duration:24s",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fish",
      heading = _"Out of Fish",
      message = pgettext("europeans_building", "The fisher working out of this fisher’s hut can’t find any fish in his work area."),
   },
}

pop_textdomain()
