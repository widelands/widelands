dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_stonecutters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Stonecutter's Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4,
   },
   return_on_dismantle = {
      log = 2,
   },

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {39, 46}
      },
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {39, 46}
      }
   },

   aihints = {
      collects_ware_from_map = "granite"
   },

   working_positions = {
      amazons_stonecutter = 1
   },

   indicate_workarea_overlaps = {
      amazons_stonecutters_hut = false,
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
         },
      },
      mine_stone = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _"cutting granite",
         actions = {
           -- This order is on purpose so that the productivity
           -- drops fast once all rocks are gone.
            "callworker=cut_granite",
            "sleep=10000"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Rocks",
      heading = _"Out of Rocks",
      message = pgettext("amazons_building", "The stonecutter working at this stonecutter's hut can’t find any rocks in his work area."),
   },
}
