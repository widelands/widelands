dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_woodcutters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Woodcutter’s Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   enhancement = "amazons_rare_trees_woodcutters_hut",

   buildcost = {
      log = 4,
   },
   return_on_dismantle = {
      log = 2,
   },

   animation_directory = dirname,
   animations = {
      idle = { hotspot = {41, 46}},
      unoccupied = { hotspot = {39, 46}}
   },

   aihints = {
      collects_ware_from_map = "log",
      supports_production_of = { "liana" },
   },

   working_positions = {
      amazons_woodcutter = 1
   },

   outputs = {
      "log"
   },

   indicate_workarea_overlaps = {
      amazons_junglepreservers_hut = true,
      amazons_woodcutters_hut = false,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _"felling trees",
         actions = {
            "return=skipped unless economy needs log",
            "callworker=harvest",
            "sleep=10000"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Trees",
      heading = _"Out of Trees",
      message = pgettext ("amazons_building", "The woodcutter working at this woodcutter’s house can’t find any trees in his work area. You should consider dismantling or destroying the building or building a forester’s house."),
      productivity_threshold = 66
   },
}
