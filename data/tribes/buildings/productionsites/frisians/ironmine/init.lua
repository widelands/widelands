dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_ironmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Iron Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "frisians_ironmine_deep",

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

   indicate_workarea_overlaps = {
      frisians_ironmine = false,
      frisians_ironmine_deep = false,
   },

   aihints = {
      mines = "iron",
      mines_percent = 50,
      prohibited_till = 1000
   },

   working_positions = {
      frisians_miner = 1
   },

   inputs = {
      { name = "ration", amount = 8 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _"mining iron",
         actions = {
            "return=skipped unless economy needs iron_ore",
            "consume=ration",
            "sleep=45000",
            "animate=working 20000",
            "mine=iron 3 50 5 20",
            "produce=iron_ore"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Iron",
      heading = _"Main Iron Vein Exhausted",
      message =
         pgettext("frisians_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}
