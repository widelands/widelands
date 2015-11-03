dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_fishers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Fisher’s Hut"),
   directory = dirname,
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
      needs_water = true,
      prohibited_till = 900
   },

   working_positions = {
      barbarians_fisher = 1
   },

   outputs = {
      "fish"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing",
         actions = {
            "sleep=18000",
            "worker=fish"
         }
      },
   },
   out_of_resource_notification = {
      title = _"Out of Fish",
      message = pgettext("barbarians_building", "The fisher working out of this fisher’s hut can’t find any fish in his work area."),
   },
}
