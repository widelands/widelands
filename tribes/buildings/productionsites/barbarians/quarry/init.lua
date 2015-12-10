dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_quarry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Quarry"),
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
         template = "idle_??",
         directory = dirname,
         hotspot = { 45, 48 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 45, 48 },
      },
      unoccupied = {
         template = "unoccupied_??",
         directory = dirname,
         hotspot = { 45, 48 },
      },
   },

   aihints = {
      forced_after = 0,
      prohibited_till = 240,
      graniteproducer = true
   },

   working_positions = {
      barbarians_stonemason = 1
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
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Rocks",
      heading = _"Out of Rocks",
      message = pgettext("barbarians_building", "The stonemason working at this quarry can’t find any rocks in his work area."),
   },
}
