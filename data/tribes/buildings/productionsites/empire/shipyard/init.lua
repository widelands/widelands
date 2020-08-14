dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Shipyard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"seafaring"},

   buildcost = {
      log = 3,
      planks = 2,
      granite = 3,
      cloth = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      cloth = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 63 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 50, 63 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 50, 63 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 50, 63 },
      },
   },

   aihints = {
      needs_water = true,
      shipyard = true,
      prohibited_till = 1050
   },

   working_positions = {
      empire_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 2 },
      { name = "planks", amount = 10 },
      { name = "cloth", amount = 4 }
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
            "construct=empire_shipconstruction worker:buildship radius:6",
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
