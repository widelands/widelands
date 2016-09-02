dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Bakery"),
   helptext_script = dirname .. "helptexts.lua",
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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 63 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
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

   -- This table is nested so we can define the order in the building's UI.
   inputs = {
      { name = "water", amount = 8 },
      { name = "cornmeal", amount = 4 },
      { name = "blackroot_flour", amount = 4 }
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
