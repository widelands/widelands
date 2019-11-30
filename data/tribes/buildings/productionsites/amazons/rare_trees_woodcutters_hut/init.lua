dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_rare_trees_woodcutters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Rare Tree Woodcutter's Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   enhancement_cost = {
      log = 1,
      granite = 1,
      rope = 1
   },
   return_on_dismantle_on_enhanced = {
      log = 1,
      rope = 1
   },

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {40, 66}
      },
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {39, 66}
      }
   },

   aihints = {
      collects_ware_from_map = "ironwood",
      supports_production_of = { "clay" },
      requires_supporters = true,
      basic_amount = 1,
   },

   working_positions = {
      amazons_woodcutter_master = 1
   },

   outputs = {
      "ironwood",
      "balsa",
      "rubber",
   },

   indicate_workarea_overlaps = {
      amazons_rare_tree_plantation = true,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=harvest_balsa",
            "call=harvest_rubber",
            "call=harvest_ironwood",
            "return=no_stats"
         },
      },
      harvest_balsa = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"harvesting balsa",
         actions = {
            -- time total: 33
            "return=skipped unless economy needs balsa",
            "callworker=harvest_balsa",
            "sleep=12000"
         },
      },
      harvest_rubber = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"harvesting rubber",
         actions = {
            -- time total: 33
            "return=skipped unless economy needs rubber",
            "callworker=harvest_rubber",
            "sleep=12000"
         },
      },
      harvest_ironwood = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"harvesting ironwood",
         actions = {
            -- time total: 33
            "return=skipped unless economy needs ironwood",
            "callworker=harvest_ironwood",
            "sleep=12000"
         },
      },
   },
}
