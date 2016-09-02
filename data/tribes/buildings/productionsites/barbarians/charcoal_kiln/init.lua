dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Charcoal Kiln"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 3,
      grout = 2,
      thatch_reed = 2
   },
   return_on_dismantle = {
      log = 2,
      grout = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 71 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 50, 71 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 50, 71 },
      },
   },

   aihints = {
      prohibited_till = 900
   },

   working_positions = {
      barbarians_charcoal_burner = 1
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
