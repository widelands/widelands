dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Charcoal Kiln"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 3,
      planks = 1
   },
   return_on_dismantle = {
      log = 2,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 47, 57 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 47, 60 },
      },
   },

   aihints = {
      prohibited_till = 1200
   },

   working_positions = {
      atlanteans_charcoal_burner = 1
   },

   -- This table is nested so we can define the order in the building's UI.
   inputs = {
      { name = "log", amount = 8 }
   },
   outputs = {
      "coal"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
         descname = _"producing coal",
         actions = {
            "sleep=30000",
            "return=skipped unless economy needs coal",
            "consume=log:6",
            "animate=working 90000", -- Charcoal fires will burn for some days in real life
            "produce=coal"
         }
      },

   },
}
