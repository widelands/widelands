dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Weaving Mill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 3,
      granite = 4,
      planks = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 3,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 65, 69 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 65, 69 },
      }
   },

   aihints = {
      forced_after = 450,
      prohibited_till = 400,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      atlanteans_weaver = 1
   },

   inputs = {
      { name = "spider_silk", amount = 8 },
      { name = "gold_thread", amount = 4 }
   },
   outputs = {
      "spidercloth",
      "tabard",
      "tabard_golden"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_spidercloth",
            "call=produce_tabard",
            "call=produce_tabard_golden",
            "return=skipped"
         }
      },
      produce_spidercloth = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving spidercloth because ...
         descname = _"weaving spidercloth",
         actions = {
            -- total time 47 sec
            "return=skipped unless economy needs spidercloth",
            "sleep=10000",
            "consume=spider_silk",
            "play_sound=sound/mill weaving 120",
            "animate=working 20000",
            "sleep=17000",
            "produce=spidercloth"
         }
      },
      produce_tabard = {
         -- TRANSLATORS: Completed/Skipped/Did not start tailoring a tabard because ...
         descname = _"tailoring a tabard",
         actions = {
            -- total time 55 sec
            "return=skipped unless economy needs tabard",
            "sleep=10000",
            "consume=spider_silk",
            "play_sound=sound/mill weaving 120",
            "animate=working 26000",
            "sleep=19000",
            "produce=tabard"
         }
      },
      produce_tabard_golden = {
         -- TRANSLATORS: Completed/Skipped/Did not start tailoring a golden tabard because ...
         descname = _"tailoring a golden tabard",
         actions = {
             -- total time 60 sec
            "return=skipped unless economy needs tabard_golden",
            "consume=spider_silk gold_thread",
            "sleep=15000",
            "play_sound=sound/mill weaving 120",
            "animate=working 30000",
            "sleep=15000",
            "produce=tabard_golden"
         }
      },
   },
}
