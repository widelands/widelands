dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Shipyard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   needs_seafaring = true,

   buildcost = {
      brick = 3,
      granite = 1,
      log = 3,
      reed = 2,
      cloth = 1
   },
   return_on_dismantle = {
      brick = 2,
      log = 2,
      reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 87},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 66},
      },
   },

   aihints = {
      needs_water = true,
      shipyard = true,
      prohibited_till = 1050
   },

   indicate_workarea_overlaps = {
      frisians_shipyard = false,
   },

   working_positions = {
      frisians_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 10 },
      { name = "cloth", amount = 6 },
      { name = "clay", amount = 3 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=ship on failure fail",
            "call=ship_preparation",
            "return=no_stats"
         }
      },
      ship = {
         -- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
         descname = _"constructing a ship",
         actions = {
            "checkmap=seafaring",
            "construct=frisians_shipconstruction buildship 6",
            "sleep=20000",
         }
      },
      ship_preparation = {
         descname = _"working",
         actions = {
            "sleep=35000",
            -- no working animation yet
         }
      },
   },
}
