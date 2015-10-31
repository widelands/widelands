dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_ironmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Iron Mine"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "barbarians_ironmine_deep",

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
      mines = "iron",
      prohibited_till =900,
      mines_percent = 30
   },

   working_positions = {
      barbarians_miner = 1
   },

   inputs = {
      ration = 6
   },
   outputs = {
      "iron_ore"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
         descname = _"mining iron",
         actions = {
            "sleep=45000",
            "return=skipped unless economy needs iron_ore",
            "consume=ration",
            "animate=working 20000",
            "mine=iron 2 33 5 17",
            "produce=iron_ore"
         }
      },
   },
   out_of_resource_notification = {
      title = _"Main Iron Vein Exhausted",
      message =
         pgettext("barbarians_building", "This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}