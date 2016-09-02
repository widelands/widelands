dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Shipyard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   needs_seafaring = true,

   buildcost = {
      log = 3,
      planks = 2,
      granite = 3,
      spidercloth = 2
   },
   return_on_dismantle = {
      log = 1,
      planks = 1,
      granite = 2,
      spidercloth = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 66 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 53, 66 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 53, 66 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 53, 66 },
      }
   },

   aihints = {
      needs_water = true,
      shipyard = true,
      prohibited_till = 1500
   },

   working_positions = {
      atlanteans_shipwright = 1
   },

   -- This table is nested so we can define the order in the building's UI.
   inputs = {
      { name = "log", amount = 2 },
      { name = "planks", amount = 10 },
      { name = "spidercloth", amount = 4 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=20000",
            "call=ship",
            "return=skipped"
         }
      },
      ship = {
         -- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
         descname = _"constructing a ship",
         actions = {
            "check_map=seafaring",
            "construct=atlanteans_shipconstruction buildship 6",
            "animate=working 35000",
            "return=completed"
         }
      },
   },
}
