dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_goldmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Gold Mine"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "empire_goldmine_deep",

   buildcost = {
      log = 4,
      planks = 2
   },
   return_on_dismantle = {
      log = 2,
      planks = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 49, 49 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 49, 49 },
         fps = 10
      },
      empty = {
         template = "empty_??",
         directory = dirname,
         hotspot = { 49, 49 },
      },
   },

   aihints = {
      mines = "gold",
      mines_percent = 50,
      prohibited_till = 1200
   },

   working_positions = {
      empire_miner = 1
   },

   inputs = {
      ration = 6,
      wine = 6
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
            "consume=ration wine",
            "animate=working 20000",
            "mine=gold 2 50 5 17",
            "produce=gold_ore",
            "animate=working 20000",
            "mine=gold 2 50 5 17",
            "produce=gold_ore"
         }
      },
   },
   out_of_resource_notification = {
		-- Translators: Short for "Out of ..." for a resource
		title = _"No Gold",
		heading = _"Main Gold Vein Exhausted",
      message =
         pgettext("empire_building", "This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}
