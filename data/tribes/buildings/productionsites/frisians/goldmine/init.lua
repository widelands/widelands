push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_goldmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Gold Mine"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement = {
      name = "frisians_goldmine_deep",
      enhancement_cost = {
         brick = 2,
         granite = 1,
         log = 1,
         reed = 2
      },
      enhancement_return_on_dismantle = {
         brick = 1,
         log = 1,
         reed = 1
      }
   },

   buildcost = {
      brick = 1,
      granite = 2,
      log = 2,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      granite = 1,
      log = 1
   },

   spritesheets = {
      idle = {
         hotspot = {27, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {27, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      empty = {
         hotspot = {27, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {27, 56}
      }
   },

   aihints = {
      prohibited_till = 1100
   },

   working_positions = {
      frisians_miner = 1
   },

   inputs = {
      { name = "ration", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _("mining gold"),
         actions = {
            -- time total: 45.4 + 20 + 3.6 = 69 sec
            "return=skipped unless economy needs gold_ore",
            "consume=ration",
            "sleep=duration:45s400ms",
            "animate=working duration:20s",
            "mine=resource_gold radius:3 yield:50% when_empty:5% experience_on_fail:20%",
            "produce=gold_ore"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Gold"),
      heading = _("Main Gold Vein Exhausted"),
      message =
         pgettext("frisians_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}

pop_textdomain()
