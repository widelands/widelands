push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_goldmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Gold Mine"),
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement = {
      name = "empire_goldmine_deep",
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
      prohibited_till = 1200
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
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _("mining gold"),
         actions = {
            "return=skipped unless economy needs gold_ore",
            "consume=ration wine",
            "sleep=duration:43s",
            "call=mine_produce",
            "call=mine_produce",
            "return=skipped"
         }
      },
      mine_produce = {
         descname = _("mining gold"),
         actions = {
            "animate=working duration:21s",
            "mine=resource_gold radius:2 yield:50% when_empty:5% experience_on_fail:17%",
            "produce=gold_ore",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=ration wine",
            "produce=gold_ore:2",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Gold"),
      heading = _("Main Gold Vein Exhausted"),
      message =
         pgettext("empire_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}

pop_textdomain()
