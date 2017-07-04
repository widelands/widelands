dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Shipyard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   --representative_image = dirname .. "representative_image.png",
   size = "medium",
   needs_seafaring = true,

   buildcost = {
      brick = 7,
      granite = 1,
      log = 2,
      thatch_reed = 4
   },
   return_on_dismantle = {
      brick = 3,
      granite = 1,
      log = 1,
      thatch_reed = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 62, 48 },
         scale = 3.26
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
            "construct=frisians_shipconstruction buildship 6",
            "animate=working 35000",
            "return=completed"
         }
      },
   },
}
