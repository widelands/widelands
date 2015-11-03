dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_goldmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Gold Mine"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "barbarians_goldmine_deep",

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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 21, 36 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 21, 36 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 21, 36 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 21, 36 },
      },
   },

   aihints = {
      mines = "gold",
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
      "gold_ore"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
         descname = _"mining gold",
         actions = {
            "sleep=45000",
            "return=skipped unless economy needs gold_ore",
            "consume=ration",
            "animate=working 20000",
            "mine=gold 2 33 5 17",
            "produce=gold_ore"
         }
      },
   },
   out_of_resource_notification = {
      title = _"Main Gold Vein Exhausted",
      message =
         pgettext("barbarians_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}
