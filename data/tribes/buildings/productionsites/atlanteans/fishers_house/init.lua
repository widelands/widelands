dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_fishers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Fisher’s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 2
   },
   return_on_dismantle = {
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 34, 42 },
      },
   },

   aihints = {
      collects_ware_from_map = "fish",
      needs_water = true,
      basic_amount = 1,
      prohibited_till = 540,
      very_weak_ai_limit = 2,
      weak_ai_limit = 4
   },

   working_positions = {
      atlanteans_fisher = 1
   },

   outputs = {
      "fish"
   },

   indicate_workarea_overlaps = {
      atlanteans_fishers_house = false,
      atlanteans_fishbreeders_house = true,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing",
         actions = {
            "callworker=fish",
            "sleep=9500"
         }
      },

   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fish",
      heading = _"Out of Fish",
      message = pgettext("atlanteans_building", "The fisher working out of this fisher’s house can’t find any fish in his work area. Remember that you can increase the number of existing fish by building a fish breeder’s house."),
   },
}
