push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_marblemine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Marble Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement = {
      name = "empire_marblemine_deep",
      enhancement_cost = {
         log = 4,
         planks = 2
      },
      enhancement_return_on_dismantle = {
         log = 2,
         planks = 1
      }
   },

   buildcost = {
      log = 4,
      planks = 2
   },
   return_on_dismantle = {
      log = 2,
      planks = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 49, 49 },
      },
      empty = {
         hotspot = { 49, 49 },
      },
   },

   spritesheets = {
      working = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 49, 49 }
      },
   },

   aihints = {
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

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=mine_granite",
            "call=mine_marble",
            "return=skipped"
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _("mining granite"),
         actions = {
            "return=skipped unless economy needs marble or economy needs granite",
            "consume=ration wine",
            "sleep=duration:18s",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_marble",
            "call=a_mine_produce_granite",
            "return=skipped"
         }
      },
      mine_marble = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining marble because ...
         descname = _("mining marble"),
         actions = {
            "return=skipped unless economy needs marble or economy needs granite",
            "consume=wine ration",
            "sleep=duration:18s",
            "call=a_mine_produce_marble",
            "call=a_mine_produce_marble",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_marble",
            "return=skipped"
         }
      },
      a_mine_produce_granite = {
         descname = _("mining granite"),
         actions = {
            "animate=working duration:10s500ms",
            "mine=resource_stones radius:2 yield:50% when_empty:5% experience_on_fail:17%",
            "produce=granite",
         }
      },
      a_mine_produce_marble = {
         descname = _("mining marble"),
         actions = {
            "animate=working duration:10s500ms",
            "mine=resource_stones radius:2 yield:50% when_empty:5% experience_on_fail:17%",
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
      title = _("No Marble"),
      heading = _("Main Marble Vein Exhausted"),
      message =
         pgettext("empire_building", "This marble mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}

pop_textdomain()
