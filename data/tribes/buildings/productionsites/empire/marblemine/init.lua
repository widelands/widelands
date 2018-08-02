dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_marblemine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Marble Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "empire_marblemine_deep",

   buildcost = {
      log = 4,
      planks = 2
   },
   return_on_dismantle = {
      log = 2,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 49, 49 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 49, 49 },
         fps = 10
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 49, 49 },
      },
   },

   aihints = {
      mines = "stones",
      mines_percent = 50,
      prohibited_till = 600,
      basic_amount = 1
   },

   working_positions = {
      empire_miner = 1
   },

   inputs = {
      { name = "ration", amount = 6 },
      { name = "wine", amount = 6 }
   },
   outputs = {
      "marble",
      "granite"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=mine_granite",
            "call=mine_marble",
            "return=skipped"
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _"mining granite",
         actions = {
            "sleep=18000",
            "return=skipped unless economy needs marble or economy needs granite",
            "consume=ration wine",
            "call=mine_produce_granite",
            "call=mine_produce_granite",
            "call=mine_produce_marble",
            "call=mine_produce_granite",
            "return=skipped"
         }
      },
      mine_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining marble because ...
         descname = _"mining marble",
         actions = {
            "sleep=18000",
            "return=skipped unless economy needs marble or economy needs granite",
            "consume=wine ration",
            "call=mine_produce_marble",
            "call=mine_produce_marble",
            "call=mine_produce_granite",
            "call=mine_produce_marble",
            "return=skipped"
         }
      },
      mine_produce_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining and producing because ...
         descname = _"mining and producing",
         actions = {
            "animate=working 10500",
            "mine=stones 2 50 5 17",
            "produce=granite",
         }
      },
      mine_produce_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining and producing because ...
         descname = _"mining and producing",
         actions = {
            "animate=working 10500",
            "mine=stones 2 50 5 17",
            "produce=marble",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Marble",
      heading = _"Main Marble Vein Exhausted",
      message =
         pgettext("empire_building", "This marble mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}
