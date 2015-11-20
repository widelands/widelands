dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Shipyard"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

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
         template = "idle_??",
         directory = dirname,
         hotspot = { 62, 48 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 62, 48 },
      },
      unoccupied = {
         template = "unoccupied_??",
         directory = dirname,
         hotspot = { 62, 48 },
      },
      working = {
         template = "working_??",
         directory = dirname,
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
      blackwood = 10,
      log = 2,
      cloth = 4
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
            "check_map=seafaring", -- TODO(GunChleoc): We should make this check on game creation as well and remove it from the allowed buildings
            "construct=barbarians_shipconstruction buildship 6",
            "animate=working 35000",
            "return=completed"
         }
      },
   },
}
