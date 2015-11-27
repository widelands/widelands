dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_quarry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Quarry"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      planks = 1
   },
   return_on_dismantle = {
      log = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 40, 45 },
      },
   },

   aihints = {
      forced_after = 240,
      prohibited_till = 240,
      graniteproducer = true
   },

   working_positions = {
      atlanteans_stonecutter = 1
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
            "return=skipped"
         },
      },
      mine_stone = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _"quarrying granite",
         actions = {
           -- This order is on purpose so that the productivity
           -- drops fast once all rocks are gone.
            "worker=cut_granite",
            "sleep=25000"
         }
      },
   },
   out_of_resource_notification = {
      title = _"Out of Rocks",
      message = pgettext("atlanteans_building", "The stonecutter working at this quarry can’t find any rocks in his work area."),
   },
}
