dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_coalmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Deep Coal Mine"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "barbarians_coalmine_deeper",

   enhancement_cost = {
      log = 4,
      granite = 2
   },
   return_on_dismantle_on_enhanced = {
      log = 2,
      granite = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 21, 37 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 21, 37 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 21, 37 },
      },
      empty = {
         template = "empty_??",
         directory = dirname,
         hotspot = { 21, 37 },
      },
   },

   aihints = {
      mines = "coal",
      mines_percent = 60
   },

   working_positions = {
      barbarians_miner = 1,
      barbarians_miner_chief = 1,
   },

   inputs = {
      snack = 6
   },
   outputs = {
      "coal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
         descname = _"mining coal",
         actions = {
            "sleep=42000",
            "return=skipped unless economy needs coal",
            "consume=snack",
            "animate=working 18000",
            "mine=coal 2 66 5 17",
            "produce=coal:2",
            "animate=working 18000",
            "mine=coal 2 66 5 17",
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