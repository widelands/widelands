dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Brewery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "frisians_mead_brewery",

   buildcost = {
      brick = 5,
      granite = 3,
      log = 3,
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
         hotspot = { 42, 51 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 42, 51 },
      },
   },

   aihints = {
      prohibited_till = 600,
      forced_after = 900
   },

   working_positions = {
      frisians_brewer = 1
   },

   inputs = {
      { name = "barley", amount = 8 },
      { name = "water", amount = 8 },
   },
   outputs = {
      "beer"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"baking bread",
         actions = {
            "sleep=20000",
            "return=skipped unless economy needs beer or workers need experience",
            "consume=water barley",
            "animate=working 25000",
            "produce=beer"
         },
      },
   },
}
