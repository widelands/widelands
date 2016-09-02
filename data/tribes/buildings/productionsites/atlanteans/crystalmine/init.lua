dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_crystalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Crystal Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
      log = 7,
      planks = 4,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 3,
      planks = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 56 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 50, 56 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 50, 56 },
      },
   },

   aihints = {
      mines = "stones",
      prohibited_till = 600
   },

   working_positions = {
      atlanteans_miner = 3
   },

   -- This table is nested so we can define the order in the building's UI.
   inputs = {
      { name = "atlanteans_bread", amount = 10 },
      { name = "smoked_fish", amount = 10 },
      { name = "smoked_meat", amount = 6 }
   },
   outputs = {
      "diamond",
      "quartz",
      "granite"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=mine_granite",
            "call=mine_quartz",
            "call=mine_diamond",
            "return=skipped"
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _"mining granite",
         actions = {
            "return=skipped unless economy needs granite or economy needs quartz or economy needs diamond",
            "sleep=45000",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=granite:2",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=granite:2",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=granite",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=quartz"
         }
      },
      mine_quartz = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining quartz because ...
         descname = _"mining quartz",
         actions = {
            "return=skipped unless economy needs granite or economy needs quartz or economy needs diamond",
            "sleep=45000",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=granite:2",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=granite quartz",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=quartz:2"
         }
      },
      mine_diamond = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining diamonds because ...
         descname = _"mining diamonds",
         actions = {
            "return=skipped unless economy needs granite or economy needs quartz or economy needs diamond",
            "sleep=45000",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=diamond",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=diamond",
            "animate=working 20000",
            "mine=stones 4 100 5 2",
            "produce=granite"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Crystal",
      heading = _"Main Crystal Vein Exhausted",
      message =
         pgettext("atlanteans_building", "This crystal mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}
