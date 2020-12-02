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
            "call=produce_spidercloth",
            "call=produce_tabard"
         }
      },
      produce_spidercloth = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving spidercloth because ...
         descname = _"weaving spidercloth",
         actions = {
            -- time total: 40 + 3.6
            "return=skipped unless economy needs spidercloth",
            "consume=spider_silk:2",
            "sleep=duration:20s",
            "playsound=sound/mill/weaving priority:90%",
            "animate=working duration:20s",
            "produce=spidercloth:2"
         }
      },
      produce_tabard = {
         -- TRANSLATORS: Completed/Skipped/Did not start tailoring a tabard because ...
         descname = _"tailoring a tabard",
         actions = {
            -- time total: 40 + 3.6
            "return=skipped unless economy needs tabard",
            "consume=spider_silk",
            "sleep=duration:20s",
            "playsound=sound/mill/weaving priority:90%",
            "animate=working duration:20s",
            "produce=tabard"
         }
      },
   },
}

pop_textdomain()
