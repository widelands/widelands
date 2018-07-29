dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_fishers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Fisher’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {49, 87},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {49, 64},
      },
   },

   aihints = {
      collects_ware_from_map = "fish",
      needs_water = true,
      prohibited_till = 490
   },

   working_positions = {
      frisians_fisher = 1
   },

   outputs = {
      "fish"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing",
         actions = {
            "sleep=16000",
            "callworker=fish"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fish",
      heading = _"Out of Fish",
      message = pgettext ("frisians_building", "The fisher working out of this fisher’s house can’t find any fish in his work area."),
   },
}
