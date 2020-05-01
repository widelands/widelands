dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_wilderness_keepers_tent",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Wilderness Keepers Tent"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      rope = 2
   },
   return_on_dismantle = {
      rope = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {49, 88},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {49, 66},
      },
   },

   aihints = {
      supports_production_of = { "fish", "meat" },
      needs_water = true,
      basic_amount = 1,
      prohibited_till = 360,
   },

   working_positions = {
      amazons_wilderness_keeper = 1
   },

   indicate_workarea_overlaps = {
      amazons_hunter_gatherers_hut = true,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=breed",
            "call=release",
         },
      },
      breed = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"breeding fish",
         actions = {
            "callworker=breed_fish",
            "sleep=52500",
         },
      },
      release = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"releasing game",
         actions = {
            "callworker=release_game",
            "sleep=52500",
         },
      },
   },
}
