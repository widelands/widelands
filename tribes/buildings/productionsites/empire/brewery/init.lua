dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Brewery"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      planks = 2,
      granite = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 39, 62 },
      },
   },

   aihints = {
      forced_after = 900,
      prohibited_till = 600,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_brewer = 1
   },

   inputs = {
      water = 7,
      wheat = 7
   },
   outputs = {
      "beer"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _"brewing beer",
         actions = {
            "sleep=30000",
            "return=skipped unless economy needs beer",
            "consume=water wheat",
            "animate=working 30000",
            "produce=beer"
         }
      },
   },
}
