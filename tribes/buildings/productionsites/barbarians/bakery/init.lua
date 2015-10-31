dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Bakery"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      blackwood = 2,
      granite = 2,
      thatch_reed = 2
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
         hotspot = { 41, 58 },
      },
      unoccupied = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 41, 58 },
      },
      working = {
         template = "idle_??", -- TODO(GunChleoc): No animation yet.
         directory = dirname,
         hotspot = { 41, 58 },
      },
   },

   aihints = {
      prohibited_till = 800
   },

   working_positions = {
      barbarians_baker = 1
   },

   inputs = {
      wheat = 6,
      water = 6
   },
   outputs = {
      "barbarians_bread"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = _"baking bread",
         actions = {
            "sleep=20000",
            "return=skipped unless economy needs barbarians_bread",
            "consume=water:3 wheat:3",
            "animate=working 20000",
            "produce=barbarians_bread",
            "animate=working 20000",
            "produce=barbarians_bread"
         }
      },
   },
}
