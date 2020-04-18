dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Shipyard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"seafaring"},

   buildcost = {
      balsa = 2,
      log = 2,
      rubber = 3,
      rope = 2,
   },
   return_on_dismantle = {
      balsa = 1,
      log = 1,
      rubber = 1,
      rope = 1,
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
      amazons_shipyard = false,
   },

   working_positions = {
      amazons_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 5 },
      { name = "balsa", amount = 5 },
      { name = "rubber", amount = 3 },
      { name = "rope", amount = 3 }
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
            "construct=amazons_shipconstruction buildship 6",
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
