dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_quarry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Quarry"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1,
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {40, 71},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {40, 53}
      }
   },

   aihints = {
      collects_ware_from_map = "granite"
   },

   working_positions = {
      frisians_stonemason = 1
   },

   indicate_workarea_overlaps = {
      frisians_quarry = false,
   },

   outputs = {
      "granite"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
           -- This order is on purpose so that the productivity
           -- drops fast once all rocks are gone.
            "call=mine_stone",
            "return=no_stats"
         },
      },
      mine_stone = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _"quarrying granite",
         actions = {
           -- This order is on purpose so that the productivity
           -- drops fast once all rocks are gone.
            "callworker=cut_granite",
            "sleep=17500"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Rocks",
      heading = _"Out of Rocks",
      message = pgettext("frisians_building", "The stonemason working at this quarry can’t find any rocks in his work area."),
   },
}
