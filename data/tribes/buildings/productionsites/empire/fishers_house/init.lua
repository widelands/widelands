dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_fishers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Fisher’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },
   return_on_dismantle = {
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 60 },
      },
   },

   aihints = {
      collects_ware_from_map = "fish",
      needs_water = true,
      prohibited_till = 410
   },

   working_positions = {
      empire_fisher = 1
   },

   outputs = {
      "fish"
   },

   indicate_workarea_overlaps = {
      empire_fishers_house = false,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing",
         actions = {
            "callworker=fish",
            "sleep=10000"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fish",
      heading = _"Out of Fish",
      message = pgettext("empire_building", "The fisher working out of this fisher’s house can’t find any fish in his work area."),
   },
}
