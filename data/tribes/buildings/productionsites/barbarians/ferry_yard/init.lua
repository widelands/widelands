dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_ferry_yard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ferry Yard"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   needs_waterways = true,

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 3,
      thatch_reed = 2
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
   },

   aihints = {},

   working_positions = {
      barbarians_shipwright = 1
   },

   inputs = {
      { name = "log", amount = 8 },
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=20000",
            "consume=log:3",
            "callworker=buildferry"
         }
      },
   },
}
