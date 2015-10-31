dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_coalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Coal Mine"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "barbarians_coalmine_deep",

   buildcost = {
      log = 4,
      granite = 2
   },
   return_on_dismantle = {
      log = 2,
      granite = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 21, 36 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 21, 36 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 21, 36 },
      },
      empty = {
         template = "empty_??",
         directory = dirname,
         hotspot = { 21, 36 },
      },
   },

   aihints = {
      mines = "coal",
      mines_percent = 30,
      prohibited_till = 1200
   },

   working_positions = {
      barbarians_miner = 1
   },

   inputs = {
      ration = 6
   },
   outputs = {
      "coal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _"mining coal",
         actions = {
            "sleep=45000",
            "return=skipped unless economy needs coal",
            "consume=ration",
            "animate=working 20000",
            "mine=coal 2 33 5 17",
            "produce=coal:2"
         }
      },
   },
   out_of_resource_notification = {
      title = _"Main Coal Vein Exhausted",
      message =
         pgettext("barbarians_building", "This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}