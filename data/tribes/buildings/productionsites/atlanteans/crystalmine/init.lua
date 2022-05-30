push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_crystalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Crystal Mine"),
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 53, 61 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 53, 61 },
      },
      empty = {
         hotspot = { 53, 61 },
      },
   },

   aihints = {
      prohibited_till = 600
   },

   working_positions = {
      atlanteans_miner = 3
   },

   inputs = {
      { name = "smoked_fish", amount = 10 },
      { name = "smoked_meat", amount = 6 },
      { name = "atlanteans_bread", amount = 10 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=mine_granite",
            "call=mine_quartz",
            "call=mine_diamond",
         }
      },
      mine_granite = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _("mining granite"),
         actions = {
            "return=skipped unless economy needs granite",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "sleep=duration:40s",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_quartz",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_granite",
         }
      },
      mine_quartz = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining quartz because ...
         descname = _("mining quartz"),
         actions = {
            "return=skipped unless economy needs quartz",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "sleep=duration:30s",
            "call=a_mine_produce_quartz",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_quartz",
            "call=a_mine_produce_quartz",
         }
      },
      mine_diamond = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining diamonds because ...
         descname = _("mining diamonds"),
         actions = {
            "return=skipped unless economy needs diamond",
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "sleep=duration:40s",
            "call=a_mine_produce_diamond",
            "call=a_mine_produce_granite",
            "call=a_mine_produce_diamond",
         }
      },
      a_mine_produce_granite = {
         descname = _("mining granite"),
         actions = {
            "animate=working duration:15s",
            "mine=resource_stones radius:4 yield:100% when_empty:5%",
            "produce=granite",
         }
      },
      a_mine_produce_quartz = {
         descname = _("mining quartz"),
         actions = {
            "animate=working duration:10s",
            "mine=resource_stones radius:4 yield:100% when_empty:5%",
            "produce=quartz",
         }
      },
      a_mine_produce_diamond = {
         descname = _("mining diamonds"),
         actions = {
            "animate=working duration:25s",
            "mine=resource_stones radius:4 yield:100% when_empty:5%",
            "produce=diamond",
         }
      },
      encyclopedia_granite = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "produce=granite:5 quartz",
         }
      },
      encyclopedia_quartz = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "produce=quartz:3 granite:3",
         }
      },
      encyclopedia_diamond = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=smoked_fish,smoked_meat:2 atlanteans_bread:2",
            "produce=diamond:2 granite",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Crystals"),
      heading = _("Main Crystal Vein Exhausted"),
      message =
         pgettext("atlanteans_building", "This crystal mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider dismantling or destroying it."),
   },
}

pop_textdomain()
