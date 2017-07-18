dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Bakery"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "frisians_honey_bread_bakery",

   buildcost = {
      brick = 5,
      granite = 4,
      log = 4,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 3,
      granite = 2,
      log = 2,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 65 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 42, 65 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 42, 65 },
      },
   },

   aihints = {
      prohibited_till = 700
   },

   working_positions = {
      frisians_baker = 1
   },

   inputs = {
      { name = "barley", amount = 6 },
      { name = "water", amount = 6 },
   },
   outputs = {
      "bread_frisians"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = _"baking bread",
         actions = {
            "sleep=10000",
            "return=skipped unless economy needs bread_frisians or workers need experience",
            "consume=water barley",
            "animate=working 40000",
            "produce=bread_frisians"
         },
      },
   },
}
