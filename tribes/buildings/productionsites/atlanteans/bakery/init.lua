dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Bakery"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      planks = 2,
      granite = 3
   },
   return_on_dismantle = {
      planks = 1,
      granite = 2
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 52, 63 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 52, 75 },
         fps = 20
      }
   },

   aihints = {
      forced_after = 1200,
      prohibited_till = 900
   },

   working_positions = {
      atlanteans_baker = 1
   },

   inputs = {
      blackroot_flour = 4,
      cornmeal = 4,
      water = 8
   },
   outputs = {
      "atlanteans_bread"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = _"baking bread",
         actions = {
            "sleep=35000",
            "return=skipped unless economy needs atlanteans_bread",
            "consume=water:2 blackroot_flour cornmeal",
            "animate=working 30000",
            "produce=atlanteans_bread:2"
         }
      },
   },
}
