dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_stonemasons_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Stonemason’s House"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1,
      marble = 3 -- Someone who works on marble should like marble.
   },
   return_on_dismantle = {
      granite = 1,
      marble = 2
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 58, 61 },
      },
      working = {
         template = "idle_??", -- TODO(GunChleoc): No animation yet.
         directory = dirname,
         hotspot = { 58, 61 },
      },
   },

   aihints = {
      forced_after = 400,
      prohibited_till = 400,
      weak_ai_limit = 1,
      normal_ai_limit = 2
   },

   working_positions = {
      empire_stonemason = 1
   },

   inputs = {
      marble = 6
   },
   outputs = {
      "marble_column"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start sculpting a marble column because ...
         descname = _"sculpting a marble column",
         actions = {
            "sleep=50000",
            "return=skipped unless economy needs marble_column",
            "consume=marble:2",
            "playFX=sound/stonecutting stonemason 192",
            "animate=working 32000",
            "produce=marble_column"
         }
      },
   },
}
