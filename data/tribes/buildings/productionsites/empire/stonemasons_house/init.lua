dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_stonemasons_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Stonemason’s House"),
   helptext_script = dirname .. "helptexts.lua",
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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 58, 61 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 58, 61 },
      },
   },

   aihints = {
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_stonemason = 1
   },

   inputs = {
      { name = "marble", amount = 6 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start sculpting a marble column because ...
         descname = _"sculpting a marble column",
         actions = {
            "return=skipped unless economy needs marble_column",
            "consume=marble:2",
            "sleep=50000",
            "playsound=sound/stonecutting/stonemason 192",
            "animate=working duration:32s",
            "produce=marble_column"
         }
      },
   },
}
