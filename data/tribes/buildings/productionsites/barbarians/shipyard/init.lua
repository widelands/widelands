dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Shipyard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   needs_seafaring = true,

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 3,
      cloth = 2
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 62, 48 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 62, 48 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 62, 48 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 62, 48 },
      },
   },

   aihints = {
      needs_water = true,
      shipyard = true,
      prohibited_till = 1500
   },

   working_positions = {
      barbarians_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 2 },
      { name = "blackwood", amount = 10 },
      { name = "cloth", amount = 4 }
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
            "construct=barbarians_shipconstruction buildship 6",
            "animate=working 35000",
            "return=completed"
         }
      },
   },
}
