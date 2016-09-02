dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Charcoal Kiln"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 2,
      marble = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 1,
      marble = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 51, 48 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 51, 48 },
      },
   },

   aihints = {
      prohibited_till = 600
   },

   working_positions = {
      empire_charcoal_burner = 1
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
