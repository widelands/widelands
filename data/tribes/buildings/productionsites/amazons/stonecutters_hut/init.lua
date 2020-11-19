push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "amazons_stonecutters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Stonecutter’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4,
   },
   return_on_dismantle = {
      log = 2,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {39, 46}},
      unoccupied = {hotspot = {39, 46}}
   },

   aihints = {
      collects_ware_from_map = "granite"
   },

   working_positions = {
      amazons_stonecutter = 1
   },

   programs = {
      main = {
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
            "sleep=duration:10s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Rocks",
      heading = _"Out of Rocks",
      message = pgettext("amazons_building", "The stonecutter working at this stonecutter’s hut can’t find any rocks in her work area."),
   },
}

pop_textdomain()
