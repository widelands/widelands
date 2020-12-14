push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_weaving_mill_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Weaving Mill"),
   icon = dirname .. "menu.png",
   size = "big",

   enhancement = {
        name = "europeans_weaving_mill_advanced",
        enhancement_cost = {
          marble_column = 3,
          quartz = 1,
          diamond = 1
        },
        enhancement_return_on_dismantle = {
          marble = 3,
          quartz = 1,
          diamond = 1
        },
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
      prohibited_till = 7200
   },

   working_positions = {
      europeans_weaver_normal = 1
   },

   inputs = {
      { name = "spider_silk", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_spidercloth_basic",
            "call=produce_spidercloth",
            "call=produce_tabard"
         }
      },
      produce_spidercloth_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
         descname = _"weaving",
         actions = {
            "sleep=duration:60s",
            "consume=spider_silk",
            "playsound=sound/mill/weaving priority:90%",
            "animate=working duration:40s",
            "produce=spidercloth"
         }
      },
      produce_spidercloth = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving spidercloth because ...
         descname = _"weaving spidercloth",
         actions = {
            "return=skipped unless economy needs spidercloth",
            "consume=spider_silk:3",
            "sleep=duration:40s",
            "playsound=sound/mill/weaving priority:90%",
            "animate=working duration:40s",
            "produce=spidercloth:3"
         }
      },
      produce_tabard = {
         -- TRANSLATORS: Completed/Skipped/Did not start tailoring a tabard because ...
         descname = _"tailoring a tabard",
         actions = {
            "return=skipped unless economy needs tabard",
            "consume=spider_silk",
            "sleep=duration:40s",
            "playsound=sound/mill/weaving priority:90%",
            "animate=working duration:40s",
            "produce=tabard"
         }
      },
   },
}

pop_textdomain()
