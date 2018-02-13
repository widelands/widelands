dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_beekeepers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Bee-keeper´s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {42, 71},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {42, 71},
      },
   },

   aihints = {
      prohibited_till = 600,
   },

   working_positions = {
      frisians_beekeeper = 1
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=45000",
            "worker=bees"
         }
      },
   },
   
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Flowers",
      heading = _"Out of Flowers",
      message = pgettext ("frisians_building", "The bee-keeper working at this bee-keepers´s house can’t find any flowering fields or bushes in his work area. You should consider dismantling or destroying the building or building a farm or berry farm."),
      productivity_threshold = 33
   },
}
