dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Shipyard"),
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"seafaring"},

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
      prohibited_till = 1050
   },

   working_positions = {
      atlanteans_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 2 },
      { name = "planks", amount = 10 },
      { name = "spidercloth", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=ship on failure fail",
            "call=ship_preparation",
         }
      },
      ship = {
         -- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
         descname = _"constructing a ship",
         actions = {
            "construct=atlanteans_shipconstruction worker:buildship radius:6",
            "sleep=duration:20s",
         }
      },
      ship_preparation = {
         descname = _"working",
         actions = {
            "animate=working duration:35s",
         }
      },
   },
}
