dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_fishers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Fisher’s Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4
   },
   return_on_dismantle = {
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 52 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 39, 52 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 39, 52 },
      },
   },

   aihints = {
      collects_ware_from_map = "fish",
      needs_water = true,
      prohibited_till = 490,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      barbarians_fisher = 1
   },

   outputs = {
      "fish"
   },

   indicate_workarea_overlaps = {
      barbarians_fishers_hut = false,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing",
         actions = {
            "callworker=fish",
            "sleep=10500"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fish",
      heading = _"Out of Fish",
      message = pgettext("barbarians_building", "The fisher working out of this fisher’s hut can’t find any fish in his work area."),
   },
}
