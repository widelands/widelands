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

   indicate_workarea_overlaps = {
      empire_marblemine = false,
      empire_marblemine_deep = false,
   },

   working_positions = {
      empire_miner = 1
   },

   inputs = {
      { name = "ration", amount = 6 },
      { name = "wine", amount = 6 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=mine_granite",
            "call=mine_marble",
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _"mining granite",
         actions = {
            "return=skipped unless economy needs marble or economy needs granite",
            "consume=ration wine",
            "sleep=18000",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_marble",
            "call=a_mine_produce_granite",
         }
      },
      mine_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining marble because ...
         descname = _"mining marble",
         actions = {
            "return=skipped unless economy needs marble or economy needs granite",
            "consume=wine ration",
            "sleep=18000",
            "call=a_mine_produce_marble",
            "call=a_mine_produce_marble",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_marble",
         }
      },
      a_mine_produce_granite = {
         descname = _"mining granite",
         actions = {
            "animate=working 10500",
            "mine=stones 2 50 5 17",
            "produce=granite",
         }
      },
      a_mine_produce_marble = {
         descname = _"mining marble",
         actions = {
            "animate=working 10500",
            "mine=stones 2 50 5 17",
            "produce=marble",
         }
      },
      encyclopedia_granite = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=ration wine",
            "produce=granite:3 marble",
         }
      },
      encyclopedia_marble = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=wine ration",
            "produce=marble:3 granite",
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
