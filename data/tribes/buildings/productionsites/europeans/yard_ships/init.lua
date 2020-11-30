push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_shipyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Shipyard"),
   icon = dirname .. "menu.png",
   size = "medium",
   map_check = {"seafaring"},

   buildcost = {
      planks = 5,
      reed = 4,
      spidercloth = 4,
      brick = 3,
      grout = 3,
      quartz = 2
   },
   return_on_dismantle = {
      log = 4,
      reed = 2,
      spidercloth = 2,
      granite = 3,
      quartz = 2
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
      prohibited_till = 3600
   },

   working_positions = {
      europeans_shipwright = 1
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
            "return=no_stats"
         }
      },
      ship = {
         -- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
         descname = _"constructing a ship",
         actions = {
            "return=skipped when not site has log",
            "return=skipped when not site has planks",
            "return=skipped when not site has spidercloth",
            "sleep=duration:10s",
            "construct=europeans_shipconstruction worker:buildship radius:6",
            "sleep=duration:10s",
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

pop_textdomain()
