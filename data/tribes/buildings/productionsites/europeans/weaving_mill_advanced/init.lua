push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_weaving_mill_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Weaving Mill"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 65, 62 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 65, 62 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 65, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 65, 62 },
         fps = 5
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_weaver_advanced = 1
   },

   inputs = {
      { name = "spider_silk", amount = 8 },
      { name = "wool", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_spidercloth",
            "call=produce_tabard",
            "call=produce_armor"
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
      produce_armor = {
         -- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
         descname = _"weaving",
         actions = {
            "return=skipped unless economy needs armor",
            "consume=wool",
            "sleep=duration:20s",
            "playsound=sound/mill/weaving priority:90%",
            "animate=working duration:15s", -- Unsure of balancing CW
            "sleep=duration:5s",
            "produce=armor"
         }
      },
   },
}

pop_textdomain()
