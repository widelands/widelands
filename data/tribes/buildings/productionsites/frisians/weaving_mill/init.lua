dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Weaving mill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   needs_seafaring = true,

   buildcost = {
      brick = 6,
      granite = 3,
      log = 2,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 3,
      granite = 2,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 85 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 85 },
      },
   },

   aihints = {
      forced_after = 900
   },

   working_positions = {
      frisians_seamstress = 1
   },

   inputs = {
      { name = "fur", amount = 6 },
      { name = "thatch_reed", amount = 6 },
   },
   outputs = {
      "cloth",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"working",
         actions = {
            "sleep=16000",
            "return=skipped unless economy needs cloth",
            "consume=fur thatch_reed",
            "animate=working 18000",
            "produce=cloth"
         },
      },
   },
}
