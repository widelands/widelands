dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_quarry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Quarry"),
   helptext_script = dirname .. "helptexts.lua",
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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 57 },
      },
   },

   aihints = {
      graniteproducer = true,
      forced_after = 240,
      prohibited_till = 240
   },

   working_positions = {
      empire_stonemason = 1
   },

   outputs = {
      "granite",
      "marble"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=mine_granite",
            "call=mine_granite",
            "call=mine_granite",
            "call=mine_marble", -- This will find marble 2 out of 7 times
            "call=mine_granite",
            "call=mine_granite",
            "call=mine_marble", -- This will find marble 2 out of 7 times
            "return=skipped"
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
         descname = _"quarrying granite",
         actions = {
           -- This order is on purpose so that the productivity
           -- drops fast once all rocks are gone.
            "worker=cut_granite",
            "sleep=25000"
         }
      },
      mine_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start quarrying marble because ...
         descname = _"quarrying marble",
         actions = {
           -- This order is on purpose so that the productivity
           -- drops fast once all rocks are gone.
            "worker=cut_marble",
            "sleep=25000"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Rocks",
      heading = _"Out of Rocks",
      message = pgettext("empire_building", "The stonemason working at this quarry can’t find any rocks in his work area."),
   },
}
