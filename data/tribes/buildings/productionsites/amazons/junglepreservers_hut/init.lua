dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_junglepreservers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("amazons_building", "Junglepreserver's Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3,
      rope = 1
   },
   return_on_dismantle = {
      log = 2,
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 46},
         fps = 4,
         frames = 4,
         columns = 2,
         rows = 2
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {39, 46}
      }
   },

   aihints = {
      supports_production_of = { "log", "liana" },
      space_consumer = true,
   },

   working_positions = {
      amazons_jungle_preserver = 1
   },

   indicate_workarea_overlaps = {
      amazons_cocoa_farm = false,
      amazons_cassava_root_plantation = false,
      amazons_rare_tree_plantation = false,
      amazons_junglepreservers_hut = false,
      amazons_liana_cutters_hut = true,
      amazons_woodcutters_hut = true,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "callworker=plant",
            "sleep=9000"
         }
      },
   },
}
