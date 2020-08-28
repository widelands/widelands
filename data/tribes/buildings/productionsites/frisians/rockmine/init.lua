dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_rockmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Rock Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement = {
      name = "frisians_rockmine_deep",
      buildcost = {
         brick = 2,
         granite = 1,
         log = 1,
         reed = 2
      },
      return_on_dismantle = {
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
         directory = dirname,
         basename = "idle",
         hotspot = {27, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {27, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      empty = {
         directory = dirname,
         basename = "empty",
         hotspot = {27, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {27, 56}
      }
   },

   aihints = {
      mines = "stones",
      mines_percent = 50,
      prohibited_till = 630
   },

   working_positions = {
      frisians_miner = 1
   },

   inputs = {
      { name = "ration", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _"mining granite",
         actions = {
            "sleep=duration:5s",
            "return=skipped unless economy needs granite",
            "consume=ration",
            "sleep=duration:38s",
            "call=mine_produce",
            "call=mine_produce",
         }
      },
      mine_produce = {
         descname = _"mining granite",
         actions = {
            "animate=working duration:21s",
            "mine=resource_stones radius:3 yield:50% when_empty:5% experience_on_fail:20%",
            "produce=granite",
         }
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=ration",
            "produce=granite:2",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Granite",
      heading = _"Main Granite Vein Exhausted",
      message =
         pgettext("frisians_building", "This rock mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}
