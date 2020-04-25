dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_hunter_gatherers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Hunter-Gatherer’s Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3,
      rope = 1
   },
   return_on_dismantle = {
      log = 1,
      rope = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {39, 46}},
      unoccupied = {hotspot = {39, 46}}
   },

   aihints = {
      collects_ware_from_map = "fish",
      needs_water = true,
      requires_supporters = true,
      --basic_amount = 1,
   },

   indicate_workarea_overlaps = {
      amazons_hunter_gatherers_hut = false,
      amazons_wilderness_keepers_tent = true
   },

   working_positions = {
      amazons_hunter_gatherer = 1
   },

   outputs = {
      "fish",
      "meat"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _"gathering",
         actions = {
            "call=hunt",
            "call=fish",
            "return=no_stats",
         }
      },
      hunt = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _"gathering meat",
         actions = {
            "callworker=hunt",
            "sleep=30000",
         }
      },
      fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _"gathering fish",
         actions = {
            "callworker=fish",
            "sleep=38500",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of Game and out of fish" for a resource
      title = _"No Game, No Fish",
      -- TRANSLATORS: "Game" means animals that you can hunt
      heading = _"Out of Game and Fish",
      -- TRANSLATORS: "game" means animals that you can hunt
      message = pgettext("amazons_building", "The hunter-gatherer working out of this hunter-gatherer’s hut can’t find any game or fish in his work area."),
      productivity_threshold = 33
   },
}
