dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_claypit",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Clay Pit"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      granite = 1,
      log = 1,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 87 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 87 },
      },
   },

   aihints = {
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1
   },

   working_positions = {
      frisians_clay_burner = 1
   },

   inputs = {
      { name = "water", amount = 4 },
   },
   outputs = {
      "clay"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start making clay because ...
         descname = _"making clay",
         actions = {
            "sleep=4000",
            "return=skipped unless economy needs clay",
            "return=failed unless site has water",
            "worker=dig",
            "consume=water",
            "animate=working 10000",
            "produce=clay"
         },
      },
   },
}
