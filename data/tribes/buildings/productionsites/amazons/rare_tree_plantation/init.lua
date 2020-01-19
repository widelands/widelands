dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_rare_tree_plantation",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Rare Tree Plantation"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 6,
      granite = 3,
      rope = 3,
   },
   return_on_dismantle = {
      log = 2,
      granite = 2,
      rope = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 82},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 66},
      },
   },

   aihints = {
      supports_production_of = { "balsa", "rubber", },
      requires_supporters = true,
      space_consumer = true,
   },

   working_positions = {
      amazons_jungle_master = 1
   },

   -- this is a dirty hack to make the link of plantation and woodcutter in the opposite direction
   -- this hack is needed due to the rare cutter being an upgraded building
   outputs = {
      "ironwood",
   },

   indicate_workarea_overlaps = {
      amazons_rare_tree_plantation = false,
      amazons_cocoa_farm = false,
      amazons_cassava_root_plantation = false,
      amazons_junglepreservers_hut = false,
      amazons_rare_trees_woodcutters_hut = true,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant_balsa",
            "call=plant_rubber",
            "call=plant_ironwood",
            "return=no_stats"
         },
      },
      plant_balsa = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"planting balsatree",
         actions = {
            -- time total: xx
            "return=skipped unless economy needs balsa",
            "callworker=plant_balsa",
            "sleep=12000"
         },
      },
      plant_rubber = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"planting rubbertree",
         actions = {
            -- time total: xx
            "return=skipped unless economy needs rubber",
            "callworker=plant_rubber",
            "sleep=12000"
         },
      },
      plant_ironwood = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing only one ration because ... (can produce more efficient when supply is good)
         descname = _"planting ironwoodtree",
         actions = {
            -- time total: xx
            "return=skipped unless economy needs ironwood",
            "callworker=plant_ironwood",
            "sleep=12000"
         },
      },
   }
}