dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_gardening_center",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Gardening Center"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 4,
      granite = 2,
      rope = 2,
   },
   return_on_dismantle = {
      log = 2,
      granite = 1,
      rope = 1,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {49, 90},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {49, 68},
      },
   },

   aihints = {
      --supports_production_of = { "balsa", "rubber", "ironwood" },
      prohibited_till = 460
   },

   working_positions = {
      amazons_jungle_master = 1
   },

   inputs = {
      { name = "charcoal", amount = 3 },
      { name = "fish", amount = 3 },
   },

   indicate_workarea_overlaps = {
      amazons_rare_tree_plantation = true,
      amazons_junglepreservers_hut = true,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting bushes because ...
         descname = _"enhancing the fertility of land",
         actions = {
            "callworker=check",
            "consume=charcoal fish",
            "call=terraform",
            "call=terraform",
            "call=terraform",
            "call=terraform",
            "call=terraform",
            "call=terraform",
         }
      },
      terraform = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting bushes because ...
         descname = _"enhancing the fertility of land",
         actions = {
            "callworker=terraform",
            "sleep=8000"
         }
      },
   },
}
